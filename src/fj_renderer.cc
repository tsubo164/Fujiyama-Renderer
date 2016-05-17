// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_renderer.h"
#include "fj_adaptive_grid_sampler.h"
#include "fj_fixed_grid_sampler.h"
#include "fj_multi_thread.h"
#include "fj_pixel_sample.h"
#include "fj_framebuffer.h"
#include "fj_rectangle.h"
#include "fj_property.h"
#include "fj_protocol.h"
#include "fj_numeric.h"
#include "fj_sampler.h"
#include "fj_shading.h"
#include "fj_camera.h"
#include "fj_filter.h"
#include "fj_socket.h"
#include "fj_vector.h"
#include "fj_light.h"
#include "fj_tiler.h"
#include "fj_ray.h"
#include "fj_box.h"

#include <vector>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cfloat>
#include <ctime>

#include <cerrno>

namespace fj {

static bool is_socket_ready = false;
static int renderer_instance_count = 0;

static int32_t generate_frame_id()
{
  const unsigned int seed = static_cast<unsigned int>(clock());

  XorShift rng(seed);
  const int32_t id = static_cast<int32_t>(rng.NextInteger());
  return id < 0 ? -id : id;
}

static void distribute_progress_iterations(FrameProgress *progress, Iteration total_iteration_count)
{
  const Iteration partial_itr = (Iteration) floor(total_iteration_count / 10.);
  Iteration remains = total_iteration_count - partial_itr * 10;
  int i;

  for (i = 0; i < 10; i++) {
    const Iteration this_itr = partial_itr + (remains > 0 ? 1 : 0);
    progress->iteration_list[i] = this_itr;

    if (remains > 0) {
      remains--;
    }
  }

  progress->current_segment = 0;
}

static void init_frame_progress(FrameProgress *progress, Iteration total_iteration_count)
{
  distribute_progress_iterations(progress, total_iteration_count);

  if (!is_socket_ready) {
    progress->report_to_viewer = false;
  }
}

static Interrupt default_frame_start(void *data, const FrameInfo *info)
{
  FrameProgress *fp = (FrameProgress *) data;

  fp->timer.Start();
  printf("# Rendering Frame\n");
  printf("#   Frame ID:     %4d\n", info->frame_id);
  printf("#   Resolution:   %4d x %d\n", info->xres, info->yres);
  printf("#   Thread Count: %4d\n", info->worker_count);
  printf("#   Tile Count:   %4d\n", info->tile_count);
  printf("\n");

  {
    int idx;
    fp->current_segment = 0;
    idx = fp->current_segment;
    fp->progress.Start(fp->iteration_list[idx]);
  }

  return CALLBACK_CONTINUE;
}
static Interrupt default_frame_done(void *data, const FrameInfo *info)
{
  FrameProgress *fp = (FrameProgress *) data;
  Elapse elapse;

  elapse = fp->timer.GetElapse();
  printf("# Frame Done\n");
  printf("#   %dh %dm %ds\n", elapse.hour, elapse.min, elapse.sec);
  printf("\n");

  return CALLBACK_CONTINUE;
}

static Interrupt default_tile_start(void *data, const TileInfo *info)
{
  return CALLBACK_CONTINUE;
}
static void increment_progress(void *data)
{
  FrameProgress *fp = reinterpret_cast<FrameProgress *>(data);
  const ProgressStatus status = fp->progress.Increment();

  if (status == PROGRESS_DONE) {
    Elapse elapse;
    int idx;

    fp->current_segment++;
    idx = fp->current_segment;
    elapse = fp->timer.GetElapse();

    printf(" %3d%%  ", idx * 10); 
    printf("(%dh %dm %ds)\n", elapse.hour, elapse.min, elapse.sec);
    fp->progress.Done();

    if (idx != 10) {
      fp->progress.Start(fp->iteration_list[idx]);
    }
  }
}
static Interrupt default_sample_done(void *data)
{
  return CALLBACK_CONTINUE;
}
static Interrupt default_tile_done(void *data, const TileInfo *info)
{
  return CALLBACK_CONTINUE;
}

// TODO TEST
static Interrupt default_frame_start2(void *data, const FrameInfo *info)
{
  FrameProgress *fp = reinterpret_cast<FrameProgress *>(data);

  if (fp->report_to_viewer) {
    Socket socket;
    socket.Open();
    socket.SetAddress("127.0.0.1");

    const int result = socket.Connect();

    if (result == -1) {
      std::cerr << "* ERROR: cannot connect to fbview: " << strerror(errno) << "\n";
      std::cerr << "*   run 'fbview --listen' if not opened yet.\n";
      std::cerr << "*   press 'l' to switch to listen mode if already opened.\n\n";
      fp->report_to_viewer = false;
    }
    else {
      SendRenderFrameStart(socket,
          info->frame_id,
          info->xres,
          info->yres,
          info->framebuffer->GetChannelCount(),
          info->tile_count);

      const int err = ReceiveEOF(socket);
      if (err) {
        // TODO ERROR HANDLING
      }
    }
  }

  fp->timer.Start();
  printf("# Rendering Frame\n");
  printf("#   Frame ID:     %4d\n", info->frame_id);
  printf("#   Resolution:   %4d x %d\n", info->xres, info->yres);
  printf("#   Thread Count: %4d\n", info->worker_count);
  printf("#   Tile Count:   %4d\n", info->tile_count);
  printf("\n");

  // start progress
  fp->current_segment = 0;
  const int idx = fp->current_segment;
  fp->progress.Start(fp->iteration_list[idx]);

  return CALLBACK_CONTINUE;
}

static Interrupt default_frame_done2(void *data, const FrameInfo *info)
{
  FrameProgress *fp = reinterpret_cast<FrameProgress *>(data);
  const Elapse elapse = fp->timer.GetElapse();

  printf("# Frame Done\n");
  printf("#   %dh %dm %ds\n", elapse.hour, elapse.min, elapse.sec);
  printf("\n");

  if (fp->report_to_viewer) {
    Socket socket;
    socket.Open();
    socket.SetAddress("127.0.0.1");

    const int result = socket.Connect();

    if (result == -1) {
      // TODO ERROR HANDLING
      std::cerr << "* ERROR: cannot connect to fbview: " << strerror(errno) << "\n";
    }

    SendRenderFrameDone(socket, info->frame_id);

    const int err = ReceiveEOF(socket);
    if (err) {
      // TODO ERROR HANDLING
    }
  }

  return CALLBACK_CONTINUE;
}

static Interrupt default_tile_start2(void *data, const TileInfo *info)
{
  FrameProgress *fp = reinterpret_cast<FrameProgress *>(data);
  if (fp->report_to_viewer == false) {
    return CALLBACK_CONTINUE;
  }

  const int MAX_RETRY = 10;
  int i = 0;

  for (i = 0; i < MAX_RETRY; i++) {
    int err = 0;
    Socket socket;
    socket.Open();
    socket.SetAddress("127.0.0.1");

    const int result = socket.Connect();
    if (result == -1) {
      // TODO ERROR HANDLING
      continue;
    }

    err = SendRenderTileStart(socket,
        info->frame_id,
        info->region_id,
        info->tile_region.min[0],
        info->tile_region.min[1],
        info->tile_region.max[0],
        info->tile_region.max[1]);
    if (err == -1) {
      // TODO ERROR HANDLING
      continue;
    }

    {
      Message message;
      const int result = ReceiveReply(socket, message);
      if (result == -1) {
        // no reply
        break;
      }
      if (message.type == MSG_RENDER_FRAME_ABORT) {
        return CALLBACK_INTERRUPT;
      }
    }
#if 0
    err = ReceiveEOF(socket);
    if (err) {
      // TODO ERROR HANDLING
      continue;
    }
#endif

    break;
  }

  if (i == MAX_RETRY) {
    // TODO ERROR HANDLING
  }

  return CALLBACK_CONTINUE;
}

static Interrupt default_tile_done2(void *data, const TileInfo *info)
{
  FrameProgress *fp = reinterpret_cast<FrameProgress *>(data);
  if (fp->report_to_viewer == false) {
    return CALLBACK_CONTINUE;
  }

  const int tile_w = info->tile_region.Size()[0];
  const int tile_h = info->tile_region.Size()[1];
  FrameBuffer tile_fb;
  tile_fb.Resize(tile_w, tile_h, info->framebuffer->GetChannelCount());

  {
    const int xoffset = info->tile_region.min[0];
    const int yoffset = info->tile_region.min[1];
    for (int y = 0; y < tile_h; y++) {
      for (int x = 0; x < tile_w; x++) {
        const int xx = x + xoffset;
        const int yy = y + yoffset;
        const Color4 color = info->framebuffer->GetColor(xx, yy);
        tile_fb.SetColor(x, y, color);
      }
    }
  }

  const int MAX_RETRY = 10;
  int i = 0;

  for (i = 0; i < MAX_RETRY; i++) {
    int err = 0;
    Socket socket;
    socket.Open();
    socket.SetAddress("127.0.0.1");

    const int result = socket.Connect();

    if (result == -1) {
      // TODO ERROR HANDLING
      continue;
    }

    err = SendRenderTileDone(socket,
        info->frame_id,
        info->region_id,
        info->tile_region.min[0],
        info->tile_region.min[1],
        info->tile_region.max[0],
        info->tile_region.max[1],
        tile_fb);
    if (err == -1) {
      // TODO ERROR HANDLING
      continue;
    }

    err = ReceiveEOF(socket);
    if (err) {
      // TODO ERROR HANDLING
    }

    break;
  }

  if (i == MAX_RETRY) {
    // TODO ERROR HANDLING
  }

  // increment progress
  MtCriticalSection(data, increment_progress);

  return CALLBACK_CONTINUE;
}

Renderer::Renderer()
{
  camera_ = NULL;
  framebuffer_ = NULL;
  target_objects_ = NULL;
  target_lights_ = NULL;
  nlights_ = 0;

  SetResolution(320, 240);
  SetPixelSamples(3, 3);
  SetTileSize(64, 64);
  SetFilterWidth(2, 2);
  SetSampleJitter(1);
  SetSampleTimeRange(0, 1);

  SetShadowEnable(1);
  SetMaxReflectDepth(3);
  SetMaxRefractDepth(3);

  SetRaymarchStep(.05);
  SetRaymarchShadowStep(.1);
  SetRaymarchReflectStep(.1);
  SetRaymarchRefractStep(.1);

  SetUseMaxThread(0);
  SetThreadCount(1);

  // TODO TEST
  if (0) {
  SetFrameReportCallback(&frame_progress_,
      default_frame_start,
      NULL,
      default_frame_done);
  } else {
  SetFrameReportCallback(&frame_progress_,
      default_frame_start2,
      NULL,
      default_frame_done2);
  }

  if (0) {
    SetTileReportCallback(&frame_progress_,
        default_tile_start,
        default_sample_done,
        default_tile_done);
  } else {
    SetTileReportCallback(&frame_progress_,
        default_tile_start2,
        default_sample_done,
        default_tile_done2);
  }

  if (renderer_instance_count == 0) {
    const int err = SocketStartup();
    if (err) {
      std::cerr << "SocketStartup() failed: " << SocketErrorMessage() << "\n\n";
      is_socket_ready = false;
    } else {
      is_socket_ready = true;
    }
  }
  renderer_instance_count++;
}

Renderer::~Renderer()
{
  if (renderer_instance_count == 1) {
    const int err = SocketCleanup();
    if (err) {
      std::cerr << "SocketCleanup() failed: " << SocketErrorMessage() << "\n\n";
      is_socket_ready = true;
    } else {
      is_socket_ready = false;
    }
  }
  renderer_instance_count--;
}

void Renderer::SetResolution(int xres, int yres)
{
  assert(xres > 0);
  assert(yres > 0);
  resolution_[0] = xres;
  resolution_[1] = yres;

  Renderer::SetRenderRegion(0, 0, xres, yres);
}

void Renderer::SetRenderRegion(int xmin, int ymin, int xmax, int ymax)
{
  assert(xmin >= 0);
  assert(ymin >= 0);
  assert(xmax >= 0);
  assert(ymax >= 0);
  assert(xmin < xmax);
  assert(ymin < ymax);

  frame_region_.min[0] = xmin;
  frame_region_.min[1] = ymin;
  frame_region_.max[0] = xmax;
  frame_region_.max[1] = ymax;
}

void Renderer::SetPixelSamples(int xrate, int yrate)
{
  assert(xrate > 0);
  assert(yrate > 0);
  pixelsamples_[0] = xrate;
  pixelsamples_[1] = yrate;
}

void Renderer::SetTileSize(int xtilesize, int ytilesize)
{
  assert(xtilesize > 0);
  assert(ytilesize > 0);
  tilesize_[0] = xtilesize;
  tilesize_[1] = ytilesize;
}

void Renderer::SetFilterWidth(float xfwidth, float yfwidth)
{
  assert(xfwidth > 0);
  assert(yfwidth > 0);
  filterwidth_[0] = xfwidth;
  filterwidth_[1] = yfwidth;
}

void Renderer::SetSampleJitter(float jitter)
{
  assert(jitter >= 0 && jitter <= 1);
  jitter_ = jitter;
}

void Renderer::SetSampleTimeRange(double start_time, double end_time)
{
  assert(start_time <= end_time);

  sample_time_start_ = start_time;
  sample_time_end_ = end_time;
}

void Renderer::SetShadowEnable(int enable)
{
  assert(enable == 0 || enable == 1);
  cast_shadow_ = enable;
}

void Renderer::SetMaxReflectDepth(int max_depth)
{
  assert(max_depth >= 0);
  max_reflect_depth_ = max_depth;
}

void Renderer::SetMaxRefractDepth(int max_depth)
{
  assert(max_depth >= 0);
  max_refract_depth_ = max_depth;
}

void Renderer::SetRaymarchStep(double step)
{
  assert(step > 0);
  raymarch_step_ = Max(step, .001);
}

void Renderer::SetRaymarchShadowStep(double step)
{
  assert(step > 0);
  raymarch_shadow_step_ = Max(step, .001);
}

void Renderer::SetRaymarchReflectStep(double step)
{
  assert(step > 0);
  raymarch_reflect_step_ = Max(step, .001);
}

void Renderer::SetRaymarchRefractStep(double step)
{
  assert(step > 0);
  raymarch_refract_step_ = Max(step, .001);
}

void Renderer::SetCamera(Camera *cam)
{
  assert(cam != NULL);
  camera_ = cam;
}

void Renderer::SetFrameBuffers(FrameBuffer *fb)
{
  assert(fb != NULL);
  framebuffer_ = fb;
}

void Renderer::SetTargetObjects(ObjectGroup *grp)
{
  assert(grp != NULL);
  target_objects_ = grp;
}

void Renderer::SetTargetLights(Light **lights, int nlights)
{
  assert(lights != NULL);
  assert(nlights > 0);
  target_lights_ = lights;
  nlights_ = nlights;
}

void Renderer::SetUseMaxThread(int use_max_thread)
{
  use_max_thread_ = (use_max_thread != 0);
}

void Renderer::SetThreadCount(int thread_count)
{
  const int max_thread_count = MtGetMaxThreadCount();

  if (thread_count < 1) {
    thread_count_ = 1;
  } else if (thread_count > max_thread_count) {
    thread_count_ = max_thread_count;
  } else {
    thread_count_ = thread_count;
  }
}

int Renderer::GetThreadCount() const
{
  const int max_thread_count = MtGetMaxThreadCount();

  if (use_max_thread_) {
    return max_thread_count;
  } else {
    return thread_count_;
  }
}

void Renderer::SetFrameReportCallback(void *data,
    FrameStartCallback frame_start,
    FrameAbortCallback frame_abort,
    FrameDoneCallback frame_done)
{
  CbSetFrameReport(&frame_report_,
      data,
      frame_start,
      frame_abort,
      frame_done);
}

void Renderer::SetTileReportCallback(void *data,
    TileStartCallback tile_start,
    SampleDoneCallback sample_done,
    TileDoneCallback tile_done)
{
  CbSetTileReport(&tile_report_,
      data,
      tile_start,
      sample_done,
      tile_done);
}

int Renderer::RenderScene()
{
  int err = 0;

  err = prepare_rendering();
  if (err) {
    /* TODO error handling */
    return -1;
  }

  err = execute_rendering();
  if (err) {
    /* TODO error handling */
    return -1;
  }

  return 0;
}

// TODO TMP REMOVE LATER
class Worker {
public:
  Worker() : camera(NULL), framebuffer(NULL), sampler(NULL) {}
  ~Worker()
  {
    delete sampler;
  }

public:
  int id;
  int region_id;
  int region_count;
  int xres, yres;
  int32_t frame_id;

  const Camera *camera;
  FrameBuffer *framebuffer;
  Sampler *sampler;
  Filter filter;
  std::vector<Sample> pixel_samples;

  TraceContext context;
  Rectangle tile_region;

  TileReport tile_report;

  const Tiler *tiler;
};
//class Worker;
static void init_worker(Worker *worker, int id,
    const Renderer *renderer, const Tiler *tiler);
static int render_frame_start(Renderer *renderer, const Tiler *tiler);
static ThreadStatus render_tile(void *data, const ThreadContext *context);
static void render_frame_done(Renderer *renderer, const Tiler *tiler);

int Renderer::prepare_rendering()
{
  int err = 0;

  err = preprocess_camera();
  if (err) {
    /* TODO error handling */
    return -1;
  }

  err = preprocess_framebuffer();
  if (err) {
    /* TODO error handling */
    return -1;
  }

  err = preprocess_lights();
  if (err) {
    /* TODO error handling */
    return -1;
  }

  return 0;
}

int Renderer::execute_rendering()
{
  const int thread_count = GetThreadCount();

  const int xres = resolution_[0];
  const int yres = resolution_[1];
  const int xtilesize = tilesize_[0];
  const int ytilesize = tilesize_[1];

  // Frame ID
  frame_id_ = generate_frame_id();

  // Tiler
  Tiler tiler;
  tiler.Divide(xres, yres, xtilesize, ytilesize);
  tiler.GenerateTiles(frame_region_);
  const int tile_count = tiler.GetTileCount();

  // Worker
  std::vector<Worker> worker_list(thread_count);
  for (std::size_t i = 0; i < worker_list.size(); i++) {
    init_worker(&worker_list[i], i, this, &tiler);
  }

  // FrameProgress
  init_frame_progress(&frame_progress_, tile_count);

  // Run sampling
  const int err = render_frame_start(this, &tiler);
  if (err) {
    return -1;
  }

  MtRunThreadLoop(&worker_list[0], render_tile, thread_count, 0, tile_count);

  render_frame_done(this, &tiler);

  return 0;
}

int Renderer::preprocess_camera() const
{
  if (camera_ == NULL)
    return -1;

  const int xres = resolution_[0];
  const int yres = resolution_[1];
  camera_->SetAspect(xres/(double)yres);

  return 0;
}

int Renderer::preprocess_framebuffer() const
{
  if (framebuffer_ == NULL)
    return -1;

  const int xres = resolution_[0];
  const int yres = resolution_[1];
  framebuffer_->Resize(xres, yres, 4);

  return 0;
}

int Renderer::preprocess_lights()
{
  const int NLIGHTS = nlights_;

  printf("# Preprocessing Lights\n");
  printf("#   Light Count: %d\n", NLIGHTS);

  Timer timer;
  timer.Start();

  for (int i = 0; i < NLIGHTS; i++) {
    Light *light = target_lights_[i];
    const int err = light->Preprocess();

    if (err) {
      /* TODO error handling */
      return -1;
    }
  }

  const Elapse elapse = timer.GetElapse();
  printf("# Preprocessing Lights Done\n");
  printf("#   %dh %dm %ds\n\n", elapse.hour, elapse.min, elapse.sec);

  return 0;
}

static void init_worker(Worker *worker, int id,
    const Renderer *renderer, const Tiler *tiler)
{
  const int xres = renderer->resolution_[0];
  const int yres = renderer->resolution_[1];
  const int xrate = renderer->pixelsamples_[0];
  const int yrate = renderer->pixelsamples_[1];
  const double xfwidth = renderer->filterwidth_[0];
  const double yfwidth = renderer->filterwidth_[1];

  worker->camera = renderer->camera_;
  worker->framebuffer = renderer->framebuffer_;
  worker->tiler = tiler;
  worker->id = id;

  worker->region_id = -1;
  worker->region_count = -1;
  worker->xres = xres;
  worker->yres = yres;

  worker->frame_id = renderer->frame_id_;

  // Sampler
  if (1) {
    worker->sampler = new FixedGridSampler();
  } else {
    worker->sampler = new AdaptiveGridSampler();
  }
  worker->sampler->SetResolution(Int2(xres, yres));
  worker->sampler->SetPixelSamples(Int2(xrate, yrate));
  worker->sampler->SetFilterWidth(Vector2(xfwidth, yfwidth));

  worker->sampler->SetJitter(renderer->jitter_);
  worker->sampler->SetSampleTimeRange(
      renderer->sample_time_start_, renderer->sample_time_end_);

  // Filter
  worker->filter.SetFilterType(FLT_GAUSSIAN, xfwidth, yfwidth);

  /* context */
  worker->context = SlCameraContext(renderer->target_objects_);
  worker->context.cast_shadow = renderer->cast_shadow_;
  worker->context.max_reflect_depth = renderer->max_reflect_depth_;
  worker->context.max_refract_depth = renderer->max_refract_depth_;
  worker->context.raymarch_step = renderer->raymarch_step_;
  worker->context.raymarch_shadow_step = renderer->raymarch_shadow_step_;
  worker->context.raymarch_reflect_step = renderer->raymarch_reflect_step_;
  worker->context.raymarch_refract_step = renderer->raymarch_refract_step_;

  /* region */
  worker->tile_region.min[0] = 0;
  worker->tile_region.max[0] = 0;
  worker->tile_region.min[1] = 0;
  worker->tile_region.max[1] = 0;

  /* interruption */
  worker->tile_report = renderer->tile_report_;
}

static void set_working_region(Worker *worker, int region_id)
{
  const Tile *tile = worker->tiler->GetTile(region_id);

  worker->region_id = region_id;
  worker->region_count = worker->tiler->GetTileCount();
  worker->tile_region.min[0] = tile->xmin;
  worker->tile_region.min[1] = tile->ymin;
  worker->tile_region.max[0] = tile->xmax;
  worker->tile_region.max[1] = tile->ymax;

  if (worker->sampler->GenerateSamples(worker->tile_region)) {
    /* TODO error handling */
  }
}

static Color4 apply_pixel_filter(Worker *worker, int x, int y)
{
  const int nsamples = worker->pixel_samples.size();
  const int xres = worker->xres;
  const int yres = worker->yres;
  const Filter &filter = worker->filter;

  Color4 pixel;
  float wgt_sum = 0.f;
  float inv_sum = 0.f;
  int i;

  for (i = 0; i < nsamples; i++) {
    const Sample &sample = worker->pixel_samples[i];
    double filtx = 0, filty = 0;
    double wgt = 0;

    filtx = xres * sample.uv.x - (x + .5);
    filty = yres * (1-sample.uv.y) - (y + .5);
    wgt = filter.Evaluate(filtx, filty);

    pixel.r += wgt * sample.data[0];
    pixel.g += wgt * sample.data[1];
    pixel.b += wgt * sample.data[2];
    pixel.a += wgt * sample.data[3];
    wgt_sum += wgt;
  }

  inv_sum = 1.f / wgt_sum;
  pixel.r *= inv_sum;
  pixel.g *= inv_sum;
  pixel.b *= inv_sum;
  pixel.a *= inv_sum;

  return pixel;
}

static void reconstruct_image(Worker *worker)
{
  FrameBuffer *fb = worker->framebuffer;
  const int xmin = worker->tile_region.min[0];
  const int ymin = worker->tile_region.min[1];
  const int xmax = worker->tile_region.max[0];
  const int ymax = worker->tile_region.max[1];
  int x, y;

  for (y = ymin; y < ymax; y++) {
    for (x = xmin; x < xmax; x++) {
      Color4 pixel;

      worker->sampler->GetSampleSetInPixel(worker->pixel_samples, x, y);
      pixel = apply_pixel_filter(worker, x, y);

      fb->SetColor(x, y, pixel);
    }
  }
}

static int render_frame_start(Renderer *renderer, const Tiler *tiler)
{
  FrameInfo info;
  info.frame_id = renderer->frame_id_;
  info.worker_count = renderer->GetThreadCount();
  info.tile_count = tiler->GetTileCount();
  info.xres = renderer->resolution_[0];
  info.yres = renderer->resolution_[1];
  info.frame_region = renderer->frame_region_;;
  info.framebuffer = renderer->framebuffer_;

  const Interrupt interrupt = CbReportFrameStart(&renderer->frame_report_, &info);
  if (interrupt == CALLBACK_INTERRUPT) {
    return -1;
  } else {
    return 0;
  }
}

static void render_frame_done(Renderer *renderer, const Tiler *tiler)
{
  FrameInfo info;
  info.frame_id = renderer->frame_id_;
  info.worker_count = renderer->GetThreadCount();
  info.tile_count = tiler->GetTileCount();
  info.xres = renderer->resolution_[0];
  info.yres = renderer->resolution_[1];
  info.frame_region = renderer->frame_region_;;
  info.framebuffer = renderer->framebuffer_;

  CbReportFrameDone(&renderer->frame_report_, &info);
}

static int render_tile_start(Worker *worker)
{
  TileInfo info;
  info.frame_id = worker->frame_id;
  info.worker_id = worker->id;
  info.region_id = worker->region_id;
  info.total_region_count = worker->region_count;
  info.tile_region = worker->tile_region;
  info.framebuffer = worker->framebuffer;

  const Interrupt interrupt = CbReportTileStart(&worker->tile_report, &info);
  if (interrupt == CALLBACK_INTERRUPT) {
    return -1;
  } else {
    return 0;
  }
}

static void render_tile_done(Worker *worker)
{
  TileInfo info;
  info.frame_id = worker->frame_id;
  info.worker_id = worker->id;
  info.region_id = worker->region_id;
  info.total_region_count = worker->region_count;
  info.tile_region = worker->tile_region;
  info.framebuffer = worker->framebuffer;

  CbReportTileDone(&worker->tile_report, &info);
}

static int integrate_samples(Worker *worker)
{
  Sample *smp = NULL;
  TraceContext cxt = worker->context;
  Ray ray;

  while ((smp = worker->sampler->GetNextSample()) != NULL) {
    Color4 C_trace;
    double t_hit = FLT_MAX;
    int hit = 0;
    int interrupted = 0;

    worker->camera->GetRay(smp->uv, smp->time, &ray);
    cxt.time = smp->time;

    hit = SlTrace(&cxt, &ray.orig, &ray.dir, ray.tmin, ray.tmax, &C_trace, &t_hit);
    if (hit) {
      smp->data[0] = C_trace.r;
      smp->data[1] = C_trace.g;
      smp->data[2] = C_trace.b;
      smp->data[3] = C_trace.a;
    } else {
      smp->data[0] = 0;
      smp->data[1] = 0;
      smp->data[2] = 0;
      smp->data[3] = 0;
    }

    interrupted = CbReportSampleDone(&worker->tile_report);
    if (interrupted) {
      printf("integrate_samples CANCELED!\n");
      return -1;
    }
  }
  return 0;
}

static ThreadStatus render_tile(void *data, const ThreadContext *context)
{
  Worker *worker_list = (Worker *) data;
  Worker *worker = &worker_list[context->thread_id];
  int interrupted = 0;

  set_working_region(worker, context->iteration_id);

  interrupted = render_tile_start(worker);
  if (interrupted) {
    return THREAD_LOOP_CANCEL;
  }

  interrupted = integrate_samples(worker);
  reconstruct_image(worker);

  render_tile_done(worker);

  if (interrupted) {
    return THREAD_LOOP_CANCEL;
  }

  return THREAD_LOOP_CONTINUE;
}

} // namespace xxx
