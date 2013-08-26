/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Renderer.h"
#include "FrameBuffer.h"
#include "Rectangle.h"
#include "Progress.h"
#include "Property.h"
#include "Numeric.h"
#include "Sampler.h"
#include "Camera.h"
#include "Filter.h"
#include "Memory.h"
#include "Vector.h"
#include "Light.h"
#include "Tiler.h"
#include "Timer.h"
#include "Ray.h"
#include "Box.h"
#include "SL.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

static Interrupt default_frame_start(void *data, const struct FrameInfo *info)
{
  struct Timer *timer = (struct Timer *) data;
  TimerStart(timer);
  printf("# Rendering Frame\n");
  printf("#   Thread Count: %4d\n", info->worker_count);
  printf("#   Tile Count:   %4d\n\n", info->tile_count);
  return CALLBACK_CONTINUE;
}
static Interrupt default_frame_done(void *data, const struct FrameInfo *info)
{
  struct Timer *timer = (struct Timer *) data;
  struct Elapse elapse;
  elapse = TimerGetElapse(timer);
  printf("# Frame Done\n");
  printf("#   %dh %dm %gs\n\n", elapse.hour, elapse.min, elapse.sec);
  return CALLBACK_CONTINUE;
}

static Interrupt default_tile_start(void *data, const struct TileInfo *info)
{
  struct Progress *progress = (struct Progress *) data;
  PrgStart(progress, info->total_sample_count);
  return CALLBACK_CONTINUE;
}
static Interrupt default_sample_done(void *data)
{
  struct Progress *progress = (struct Progress *) data;
  PrgIncrement(progress);
  return CALLBACK_CONTINUE;
}
static Interrupt default_tile_done(void *data, const struct TileInfo *info)
{
  struct Progress *progress = (struct Progress *) data;

  printf(" Tile Done: %d/%d (%d %%)\n",
      info->region_id + 1,
      info->total_region_count,
      (int) ((info->region_id + 1) / (double) info->total_region_count * 100));

  PrgDone(progress);
  return CALLBACK_CONTINUE;
}

struct Renderer {
  struct Camera *camera;
  struct FrameBuffer *framebuffers;
  struct ObjectGroup *target_objects;
  struct Light **target_lights;
  int nlights;

  int resolution[2];
  struct Rectangle render_region;
  int pixelsamples[2];
  int tilesize[2];
  float filterwidth[2];
  float jitter;
  double sample_time_start;
  double sample_time_end;

  int cast_shadow;
  int max_reflect_depth;
  int max_refract_depth;

  double raymarch_step;
  double raymarch_shadow_step;
  double raymarch_reflect_step;
  double raymarch_refract_step;

  struct FrameReport frame_report;
  struct TileReport tile_report;
  struct Timer frame_timer;
  struct Progress *progress;
};

static int prepare_render(struct Renderer *renderer);
static int render_scene(struct Renderer *renderer);
static int preprocess_lights(struct Renderer *renderer);

struct Renderer *RdrNew(void)
{
  struct Renderer *renderer = MEM_ALLOC(struct Renderer);
  if (renderer == NULL)
    return NULL;

  renderer->camera = NULL;
  renderer->framebuffers = NULL;
  renderer->target_objects = NULL;
  renderer->target_lights = NULL;
  renderer->nlights = 0;

  RdrSetResolution(renderer, 320, 240);
  RdrSetPixelSamples(renderer, 3, 3);
  RdrSetTileSize(renderer, 64, 64);
  RdrSetFilterWidth(renderer, 2, 2);
  RdrSetSampleJitter(renderer, 1);
  RdrSetSampleTimeRange(renderer, 0, 1);

  RdrSetShadowEnable(renderer, 1);
  RdrSetMaxReflectDepth(renderer, 3);
  RdrSetMaxRefractDepth(renderer, 3);

  RdrSetRaymarchStep(renderer, .05);
  RdrSetRaymarchShadowStep(renderer, .1);
  RdrSetRaymarchReflectStep(renderer, .1);
  RdrSetRaymarchRefractStep(renderer, .1);

  /* TODO TEST INTERRUPT */
  RdrSetFrameReportCallback(renderer, &renderer->frame_timer,
      default_frame_start,
      default_frame_done);

  renderer->progress = PrgNew();
  RdrSetTileReportCallback(renderer, renderer->progress,
      default_tile_start,
      default_sample_done,
      default_tile_done);

  return renderer;
}

void RdrFree(struct Renderer *renderer)
{
  if (renderer == NULL)
    return;
  /* TODO TEST INTERRUPT */
  PrgFree(renderer->progress);
  MEM_FREE(renderer);
}

void RdrSetResolution(struct Renderer *renderer, int xres, int yres)
{
  assert(xres > 0);
  assert(yres > 0);
  renderer->resolution[0] = xres;
  renderer->resolution[1] = yres;

  RdrSetRenderRegion(renderer, 0, 0, xres, yres);
}

void RdrSetRenderRegion(struct Renderer *renderer, int xmin, int ymin, int xmax, int ymax)
{
  assert(xmin >= 0);
  assert(ymin >= 0);
  assert(xmax >= 0);
  assert(ymax >= 0);
  assert(xmin < xmax);
  assert(ymin < ymax);

  renderer->render_region.xmin = xmin;
  renderer->render_region.ymin = ymin;
  renderer->render_region.xmax = xmax;
  renderer->render_region.ymax = ymax;
}

void RdrSetPixelSamples(struct Renderer *renderer, int xrate, int yrate)
{
  assert(xrate > 0);
  assert(yrate > 0);
  renderer->pixelsamples[0] = xrate;
  renderer->pixelsamples[1] = yrate;
}

void RdrSetTileSize(struct Renderer *renderer, int xtilesize, int ytilesize)
{
  assert(xtilesize > 0);
  assert(ytilesize > 0);
  renderer->tilesize[0] = xtilesize;
  renderer->tilesize[1] = ytilesize;
}

void RdrSetFilterWidth(struct Renderer *renderer, float xfwidth, float yfwidth)
{
  assert(xfwidth > 0);
  assert(yfwidth > 0);
  renderer->filterwidth[0] = xfwidth;
  renderer->filterwidth[1] = yfwidth;
}

void RdrSetSampleJitter(struct Renderer *renderer, float jitter)
{
  assert(jitter >= 0 && jitter <= 1);
  renderer->jitter = jitter;
}

void RdrSetSampleTimeRange(struct Renderer *renderer, double start_time, double end_time)
{
  assert(start_time <= end_time);

  renderer->sample_time_start = start_time;
  renderer->sample_time_end = end_time;
}

void RdrSetShadowEnable(struct Renderer *renderer, int enable)
{
  assert(enable == 0 || enable == 1);
  renderer->cast_shadow = enable;
}

void RdrSetMaxReflectDepth(struct Renderer *renderer, int max_depth)
{
  assert(max_depth >= 0);
  renderer->max_reflect_depth = max_depth;
}

void RdrSetMaxRefractDepth(struct Renderer *renderer, int max_depth)
{
  assert(max_depth >= 0);
  renderer->max_refract_depth = max_depth;
}

void RdrSetRaymarchStep(struct Renderer *renderer, double step)
{
  assert(step > 0);
  renderer->raymarch_step = MAX(step, .001);
}

void RdrSetRaymarchShadowStep(struct Renderer *renderer, double step)
{
  assert(step > 0);
  renderer->raymarch_shadow_step = MAX(step, .001);
}

void RdrSetRaymarchReflectStep(struct Renderer *renderer, double step)
{
  assert(step > 0);
  renderer->raymarch_reflect_step = MAX(step, .001);
}

void RdrSetRaymarchRefractStep(struct Renderer *renderer, double step)
{
  assert(step > 0);
  renderer->raymarch_refract_step = MAX(step, .001);
}

void RdrSetCamera(struct Renderer *renderer, struct Camera *cam)
{
  assert(cam != NULL);
  renderer->camera = cam;
}

void RdrSetFrameBuffers(struct Renderer *renderer, struct FrameBuffer *fb)
{
  assert(fb != NULL);
  renderer->framebuffers = fb;
}

void RdrSetTargetObjects(struct Renderer *renderer, struct ObjectGroup *grp)
{
  assert(grp != NULL);
  renderer->target_objects = grp;
}

void RdrSetTargetLights(struct Renderer *renderer, struct Light **lights, int nlights)
{
  assert(lights != NULL);
  assert(nlights > 0);
  renderer->target_lights = lights;
  renderer->nlights = nlights;
}

int RdrRender(struct Renderer *renderer)
{
  int err = 0;

  err = prepare_render(renderer);
  if (err) {
    /* TODO error handling */
    return -1;
  }

  err = render_scene(renderer);
  if (err) {
    /* TODO error handling */
    return -1;
  }

  return 0;
}

/* TODO TEST INTERRUPT */
void RdrSetFrameReportCallback(struct Renderer *renderer, void *data,
    FrameStartCallback frame_start,
    FrameDoneCallback frame_done)
{
  renderer->frame_report.data = data;
  renderer->frame_report.start = frame_start;
  renderer->frame_report.done = frame_done;
}

void RdrSetTileReportCallback(struct Renderer *renderer, void *data,
    TileStartCallback tile_start,
    SampleDoneCallback sample_done,
    TileDoneCallback tile_done)
{
  renderer->tile_report.data = data;
  renderer->tile_report.start = tile_start;
  renderer->tile_report.sample_done = sample_done;
  renderer->tile_report.done = tile_done;
}

static int prepare_render(struct Renderer *renderer)
{
  int xres, yres;
  struct Camera *cam = renderer->camera;
  struct FrameBuffer *fb = renderer->framebuffers;

  if (cam == NULL) {
    /* TODO error handling */
    return -1;
  }
  if (fb == NULL) {
    /* TODO error handling */
    return -1;
  }

  /* Preparing Camera */
  xres = renderer->resolution[0];
  yres = renderer->resolution[1];
  CamSetAspect(cam, xres/(double)yres);

  /* Prepare framebuffers */
  FbResize(fb, xres, yres, 4);

  return 0;
}

static int preprocess_lights(struct Renderer *renderer)
{
  struct Timer timer;
  struct Elapse elapse;
  const int NLIGHTS = renderer->nlights;
  int i;

  printf("# Preprocessing Lights\n");
  printf("#   Light Count: %d\n", NLIGHTS);
  TimerStart(&timer);

  for (i = 0; i < NLIGHTS; i++) {
    struct Light *light = renderer->target_lights[i];
    const int err = LgtPreprocess(light);

    if (err) {
      /* TODO error handling */
      return -1;
    }
  }

  elapse = TimerGetElapse(&timer);
  printf("# Preprocessing Lights Done\n");
  printf("#   %dh %dm %gs\n\n", elapse.hour, elapse.min, elapse.sec);

  return 0;
}

/* TODO TEST */
struct Worker {
  int id;
  int region_id;
  int region_count;
  int xres, yres;

  struct Camera *camera;
  struct FrameBuffer *framebuffer;
  struct Progress *progress;
  struct Sampler *sampler;
  struct Filter *filter;
  struct Sample *pixel_samples;

  struct TraceContext context;
  struct Rectangle region;

  /* TODO TEST INTERRUPT */
  struct TileReport tile_report;
};

void init_worker(struct Worker *worker, struct Renderer *renderer)
{
  const int xres = renderer->resolution[0];
  const int yres = renderer->resolution[1];
  const int xrate = renderer->pixelsamples[0];
  const int yrate = renderer->pixelsamples[1];
  const double xfwidth = renderer->filterwidth[0];
  const double yfwidth = renderer->filterwidth[1];

  worker->camera = renderer->camera;
  worker->framebuffer = renderer->framebuffers;

  worker->region_id = -1;
  worker->region_count = -1;
  worker->xres = xres;
  worker->yres = yres;

  /* Progress */
  worker->progress = PrgNew();
  if (worker->progress == NULL) {
    /*
    render_state = -1;
    goto cleanup_and_exit;
    */
  }

  /* Sampler */
  worker->sampler = SmpNew(xres, yres, xrate, yrate, xfwidth, yfwidth);
  if (worker->sampler == NULL) {
    /*
    render_state = -1;
    goto cleanup_and_exit;
    */
  }
  SmpSetJitter(worker->sampler, renderer->jitter);
  SmpSetSampleTimeRange(worker->sampler,
      renderer->sample_time_start, renderer->sample_time_end);
  worker->pixel_samples = SmpAllocatePixelSamples(worker->sampler);

  /* Filter */
  worker->filter = FltNew(FLT_GAUSSIAN, xfwidth, yfwidth);
  if (worker->filter == NULL) {
    /*
    render_state = -1;
    goto cleanup_and_exit;
    */
  }

  /* context */
  worker->context = SlCameraContext(renderer->target_objects);
  worker->context.cast_shadow = renderer->cast_shadow;
  worker->context.max_reflect_depth = renderer->max_reflect_depth;
  worker->context.max_refract_depth = renderer->max_refract_depth;
  worker->context.raymarch_step = renderer->raymarch_step;
  worker->context.raymarch_shadow_step = renderer->raymarch_shadow_step;
  worker->context.raymarch_reflect_step = renderer->raymarch_reflect_step;
  worker->context.raymarch_refract_step = renderer->raymarch_refract_step;

  /* region */
  worker->region.xmin = 0;
  worker->region.xmax = 0;
  worker->region.ymin = 0;
  worker->region.ymax = 0;

  /* interruption */
  worker->tile_report = renderer->tile_report;
}

void set_working_region(struct Worker *worker, struct Tiler *tiler, int region_id)
{
  struct Tile *tile = TlrGetTile(tiler, region_id);

  worker->region_id = region_id;
  worker->region_count = TlrGetTileCount(tiler);
  worker->region.xmin = tile->xmin;
  worker->region.ymin = tile->ymin;
  worker->region.xmax = tile->xmax;
  worker->region.ymax = tile->ymax;

  if (SmpGenerateSamples(worker->sampler, &worker->region)) {
    /* TODO error handling */
  }
}

void finish_worker(struct Worker *worker)
{
  SmpFreePixelSamples(worker->pixel_samples);
  PrgFree(worker->progress);
  SmpFree(worker->sampler);
  FltFree(worker->filter);
}

static struct Color4 apply_pixel_filter(struct Worker *worker, int x, int y)
{
  const int nsamples = SmpGetSampleCountForPixel(worker->sampler);
  const int xres = worker->xres;
  const int yres = worker->yres;
  struct Sample *pixel_samples = worker->pixel_samples;
  struct Filter *filter = worker->filter;

  struct Color4 pixel = {0, 0, 0, 0};
  float wgt_sum = 0.f;
  float inv_sum = 0.f;
  int i;

  for (i = 0; i < nsamples; i++) {
    struct Sample *sample = &pixel_samples[i];
    double filtx = 0, filty = 0;
    double wgt = 0;

    filtx = xres * sample->uv.x - (x + .5);
    filty = yres * (1-sample->uv.y) - (y + .5);
    wgt = FltEvaluate(filter, filtx, filty);

    pixel.r += wgt * sample->data[0];
    pixel.g += wgt * sample->data[1];
    pixel.b += wgt * sample->data[2];
    pixel.a += wgt * sample->data[3];
    wgt_sum += wgt;
  }

  inv_sum = 1.f / wgt_sum;
  pixel.r *= inv_sum;
  pixel.g *= inv_sum;
  pixel.b *= inv_sum;
  pixel.a *= inv_sum;

  return pixel;
}

static void reconstruct_image(struct Worker *worker, struct FrameBuffer *fb)
{
  const int xmin = worker->region.xmin;
  const int ymin = worker->region.ymin;
  const int xmax = worker->region.xmax;
  const int ymax = worker->region.ymax;
  int x, y;

  for (y = ymin; y < ymax; y++) {
    for (x = xmin; x < xmax; x++) {
      struct Color4 pixel = {0, 0, 0, 0};

      SmpGetPixelSamples(worker->sampler, worker->pixel_samples, x, y);
      pixel = apply_pixel_filter(worker, x, y);

      FbSetColor(fb, x, y, &pixel);
    }
  }
}

static void frame_start(struct Renderer *renderer, const struct Tiler *tiler)
{
  struct FrameInfo info;
  info.worker_count = 1;
  info.tile_count = TlrGetTileCount(tiler);
  info.render_region = renderer->render_region;;

  renderer->frame_report.start(renderer->frame_report.data, &info);
}

static void frame_done(struct Renderer *renderer, const struct Tiler *tiler)
{
  struct FrameInfo info;
  info.worker_count = 1;
  info.tile_count = TlrGetTileCount(tiler);
  info.render_region = renderer->render_region;;

  renderer->frame_report.done(renderer->frame_report.data, &info);
}

static void work_start(struct Worker *worker)
{
  struct TileInfo info;
  info.worker_id = worker->id;
  info.region_id = worker->region_id;
  info.total_region_count = worker->region_count;
  info.total_sample_count = SmpGetSampleCount(worker->sampler);
  info.region = worker->region;

  worker->tile_report.start(worker->tile_report.data, &info);
}

static void work_done(struct Worker *worker)
{
  struct TileInfo info;
  info.worker_id = worker->id;
  info.region_id = worker->region_id;
  info.total_region_count = worker->region_count;
  info.total_sample_count = SmpGetSampleCount(worker->sampler);
  info.region = worker->region;

  worker->tile_report.done(worker->tile_report.data, &info);
}

static int integrate_samples(struct Worker *worker)
{
  struct Sample *smp = NULL;
  struct TraceContext cxt = worker->context;
  struct Ray ray;

  while ((smp = SmpGetNextSample(worker->sampler)) != NULL) {
    struct Color4 C_trace = {0, 0, 0, 0};
    double t_hit = FLT_MAX;
    int hit = 0;
    int interrupted = 0;

    CamGetRay(worker->camera, &smp->uv, smp->time, &ray);
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

    /* TODO TEST INTERRUPT */
    interrupted = worker->tile_report.sample_done(worker->tile_report.data);
    if (interrupted) {
      return -1;
    }
  }
  return 0;
}

static int render_scene(struct Renderer *renderer)
{
  struct FrameBuffer *fb = renderer->framebuffers;
  struct Tiler *tiler = NULL;

  int render_state = 0;
  int err = 0;
  int i;

  const int xres = renderer->resolution[0];
  const int yres = renderer->resolution[1];
  const int xtilesize = renderer->tilesize[0];
  const int ytilesize = renderer->tilesize[1];

  struct Worker worker[1];
  init_worker(&worker[0], renderer);

  /* preprocessing lights */
  err = preprocess_lights(renderer);
  if (err) {
    render_state = -1;
    goto cleanup_and_exit;
  }

  /* Tiler */
  tiler = TlrNew(xres, yres, xtilesize, ytilesize);
  if (tiler == NULL) {
    render_state = -1;
    goto cleanup_and_exit;
  }
  TlrGenerateTiles(tiler, &renderer->render_region);

  /* Run sampling */
  frame_start(renderer, tiler);

  for (i = 0; i < TlrGetTileCount(tiler); i++) {
    int interrupted = 0;

    set_working_region(&worker[0], tiler, i);

    work_start(&worker[0]);

    interrupted = integrate_samples(&worker[0]);
    reconstruct_image(&worker[0], fb);

    work_done(&worker[0]);

    if (interrupted) {
      break;
    }
  }

  frame_done(renderer, tiler);

cleanup_and_exit:
  TlrFree(tiler);

  finish_worker(&worker[0]);

  return render_state;
}
