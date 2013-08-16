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

struct Renderer {
	struct Camera *camera;
	struct FrameBuffer *framebuffers;
	struct ObjectGroup *target_objects;
	struct Light **target_lights;
	int nlights;

	int resolution[2];
	int render_region[4];
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
};

static int prepare_render(struct Renderer *renderer);
static int render_scene(struct Renderer *renderer);
static int preprocess_lights(struct Renderer *renderer);

/* TODO TEST */
int render_scene__(struct Renderer *renderer);
static struct Color4 apply_pixel_filter(const struct Filter *filter,
    struct Sample *pixel_samples, int nsamples,
    int xres, int yres, int x, int y);
static void reconstruct_image(
    struct FrameBuffer *fb,
    struct Sampler *sampler,
    const struct Filter *filter,
    struct Sample *pixel_samples,
    int xres, int yres,
    const int *pixel_bounds);

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

	return renderer;
}

void RdrFree(struct Renderer *renderer)
{
	if (renderer == NULL)
		return;
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

	BOX2_SET(renderer->render_region, xmin, ymin, xmax, ymax);
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

  if (0)
    err = render_scene(renderer);
  else
    err = render_scene__(renderer);
	if (err) {
		/* TODO error handling */
		return -1;
	}

	return 0;
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

static int render_scene(struct Renderer *renderer)
{
	struct Camera *cam = renderer->camera;
	struct FrameBuffer *fb = renderer->framebuffers;
	struct ObjectGroup *target_objects = renderer->target_objects;

	/* context */
	struct TraceContext cxt;

/*
	int x, y;
*/
	struct Sample *pixelsmps = NULL;
	struct Sample *smp = NULL;
	struct Ray ray;
	struct Timer timer;
	struct Elapse elapse;
	struct Progress *progress = NULL;

	/* aux */
	struct Sampler *sampler = NULL;
	struct Filter *filter = NULL;
	struct Tiler *tiler = NULL;
	struct Tile *tile = NULL;

	int region[4] = {0};
	int render_state = 0;
	int err = 0;

	const int xres = renderer->resolution[0];
	const int yres = renderer->resolution[1];
	const int xrate = renderer->pixelsamples[0];
	const int yrate = renderer->pixelsamples[1];
	const double xfwidth = renderer->filterwidth[0];
	const double yfwidth = renderer->filterwidth[1];
	const int xtilesize = renderer->tilesize[0];
	const int ytilesize = renderer->tilesize[1];

	/* Progress */
	progress = PrgNew();
	if (progress == NULL) {
		render_state = -1;
		goto cleanup_and_exit;
	}

	/* Sampler */
	sampler = SmpNew(xres, yres, xrate, yrate, xfwidth, yfwidth);
	if (sampler == NULL) {
		render_state = -1;
		goto cleanup_and_exit;
	}
	SmpSetJitter(sampler, renderer->jitter);
	SmpSetSampleTimeRange(sampler, renderer->sample_time_start, renderer->sample_time_end);

	/* Filter */
	filter = FltNew(FLT_GAUSSIAN, xfwidth, yfwidth);
	if (filter == NULL) {
		render_state = -1;
		goto cleanup_and_exit;
	}

	/* Tiler */
	tiler = TlrNew(xres, yres, xtilesize, ytilesize);
	if (tiler == NULL) {
		render_state = -1;
		goto cleanup_and_exit;
	}

	/* context */
	cxt = SlCameraContext(target_objects);
	cxt.cast_shadow = renderer->cast_shadow;
	cxt.max_reflect_depth = renderer->max_reflect_depth;
	cxt.max_refract_depth = renderer->max_refract_depth;
	cxt.raymarch_step = renderer->raymarch_step;
	cxt.raymarch_shadow_step = renderer->raymarch_shadow_step;
	cxt.raymarch_reflect_step = renderer->raymarch_reflect_step;
	cxt.raymarch_refract_step = renderer->raymarch_refract_step;

	/* region */
	BOX2_COPY(region, renderer->render_region);
	TlrGenerateTiles(tiler, region[0], region[1], region[2], region[3]);

	/* samples for a pixel */
	pixelsmps = SmpAllocatePixelSamples(sampler);

	/* preprocessing lights */
	printf("Preprocessing lights ...\n");
	TimerStart(&timer);
	err = preprocess_lights(renderer);
	if (err) {
		render_state = -1;
		goto cleanup_and_exit;
	}
	elapse = TimerGetElapse(&timer);
	printf("Done: %dh %dm %gs\n", elapse.hour, elapse.min, elapse.sec);

	/* Run sampling */
	TimerStart(&timer);
	printf("Rendering ...\n");
	while ((tile = TlrGetNextTile(tiler)) != NULL) {
		int pixel_bounds[4] = {0};
		pixel_bounds[0] = tile->xmin;
		pixel_bounds[1] = tile->ymin;
		pixel_bounds[2] = tile->xmax;
		pixel_bounds[3] = tile->ymax;
		if (SmpGenerateSamples(sampler, pixel_bounds)) {
			render_state = -1;
			goto cleanup_and_exit;
		}

		PrgStart(progress, SmpGetSampleCount(sampler));

		while ((smp = SmpGetNextSample(sampler)) != NULL) {
			struct Color4 C_trace = {0, 0, 0, 0};
			double t_hit = FLT_MAX;
			int hit = 0;

			CamGetRay(cam, &smp->uv, smp->time, &ray);
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

			PrgIncrement(progress);
		}

    reconstruct_image(
        fb,
        sampler,
        filter,
        pixelsmps,
        xres, yres,
        pixel_bounds);

#if 0
		for (y = pixel_bounds[1]; y < pixel_bounds[3]; y++) {
			for (x = pixel_bounds[0]; x < pixel_bounds[2]; x++) {
				const int nsamples = SmpGetSampleCountForPixel(sampler);
        struct Color4 pixel = {0, 0, 0, 0};
				SmpGetPixelSamples(sampler, pixelsmps, x, y);

        /* TODO remove xres, yres, x, y parameters by pre-computing weights */
        pixel = apply_pixel_filter(filter, pixelsmps, nsamples,
            xres, yres, x, y);

				FbSetColor(fb, x, y, &pixel);
#if 0
				const int NSAMPLES = SmpGetSampleCountForPixel(sampler);
        struct Color4 pixel = {0, 0, 0, 0};
				float sum = 0;
				int i;

				SmpGetPixelSamples(sampler, pixelsmps, x, y);

				for (i = 0; i < NSAMPLES; i++) {
					struct Sample *sample = pixelsmps + i;
					double filtx, filty;
					double wgt;

					filtx = xres * sample->uv.x - (x + .5);
					filty = yres * (1-sample->uv.y) - (y + .5);
					wgt = FltEvaluate(filter, filtx, filty);
					pixel.r += wgt * sample->data[0];
					pixel.g += wgt * sample->data[1];
					pixel.b += wgt * sample->data[2];
					pixel.a += wgt * sample->data[3];
					sum += wgt;
				}
				{
					const float inv_sum = 1.f / sum;
					pixel.r *= inv_sum;
					pixel.g *= inv_sum;
					pixel.b *= inv_sum;
					pixel.a *= inv_sum;
				}

				FbSetColor(fb, x, y, &pixel);
#endif
			}
		}
#endif
		printf(" Tile Done: %d/%d (%d %%)\n",
				tile->id+1,
				TlrGetTileCount(tiler),
				(int) ((tile->id+1) / (double) TlrGetTileCount(tiler) * 100));
		PrgDone(progress);
	}
	elapse = TimerGetElapse(&timer);
	printf("Done: %dh %dm %gs\n", elapse.hour, elapse.min, elapse.sec);

cleanup_and_exit:
	SmpFreePixelSamples(pixelsmps);
	PrgFree(progress);
	SmpFree(sampler);
	FltFree(filter);
	TlrFree(tiler);

	return render_state;
}

static int preprocess_lights(struct Renderer *renderer)
{
	int N = 0;
	int i;

	N = renderer->nlights;
	for (i = 0; i < N; i++) {
		struct Light *light = renderer->target_lights[i];
		const int err = LgtPreprocess(light);

		if (err) {
			/* TODO error handling */
			return -1;
		}
	}

	return 0;
}

static struct Color4 apply_pixel_filter(const struct Filter *filter,
    struct Sample *pixel_samples, int nsamples,
    int xres, int yres, int x, int y)
{
  struct Color4 pixel = {0, 0, 0, 0};
  float sum = 0.f, inv_sum = 0.f;
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
    sum += wgt;
  }

  inv_sum = 1.f / sum;
  pixel.r *= inv_sum;
  pixel.g *= inv_sum;
  pixel.b *= inv_sum;
  pixel.a *= inv_sum;

  return pixel;
}

static void reconstruct_image(
    struct FrameBuffer *fb,
    struct Sampler *sampler,
    const struct Filter *filter,
    struct Sample *pixel_samples,
    int xres, int yres,
    const int *pixel_bounds)
{
#if 0
#endif
  int x, y;
  /*
  int xres, yres;
  */
  struct Sample *pixelsmps = SmpAllocatePixelSamples(sampler);
  /*
  */

  for (y = pixel_bounds[1]; y < pixel_bounds[3]; y++) {
    for (x = pixel_bounds[0]; x < pixel_bounds[2]; x++) {
      const int nsamples = SmpGetSampleCountForPixel(sampler);
      struct Color4 pixel = {0, 0, 0, 0};
      SmpGetPixelSamples(sampler, pixelsmps, x, y);

      /* TODO remove xres, yres, x, y parameters by pre-computing weights */
      pixel = apply_pixel_filter(filter, pixelsmps, nsamples,
          xres, yres, x, y);

      FbSetColor(fb, x, y, &pixel);
    }
  }

	SmpFreePixelSamples(pixelsmps);
  /*
  */
}

/* TODO TEST */
struct Worker {
	struct Camera *camera;
	struct FrameBuffer *framebuffer;
  struct Progress *progress;
  struct Sampler *sampler;
	struct Filter *filter;
	struct Sample *pixel_samples;

	struct TraceContext context;
  struct Rectangle region;

  int xres, yres;
};
#define WORKER_INIT {NULL,NULL,NULL,NULL,0,0,{0,0,0,0}}
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
}

void set_working_region(struct Worker *worker, const int *region)
{
	worker->region.xmin = region[0];
	worker->region.ymin = region[1];
	worker->region.xmax = region[2];
	worker->region.ymax = region[3];
}

void finish_worker(struct Worker *worker)
{
	SmpFreePixelSamples(worker->pixel_samples);
	PrgFree(worker->progress);
	SmpFree(worker->sampler);
	FltFree(worker->filter);
}

static struct Color4 apply_pixel_filter__(struct Worker *worker, int x, int y)
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

static void reconstruct_image__(
    struct Worker *worker,
    struct FrameBuffer *fb)
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
      pixel = apply_pixel_filter__(worker, x, y);

      FbSetColor(fb, x, y, &pixel);
    }
  }
}

static void work_start(struct Worker *worker)
{
		PrgStart(worker->progress, SmpGetSampleCount(worker->sampler));
}

static void work_doen(struct Worker *worker)
{
		PrgDone(worker->progress);
}

static void integrate_samples(struct Worker *worker)
{
  struct Sample *smp = NULL;
  struct TraceContext cxt = worker->context;
	struct Ray ray;

  while ((smp = SmpGetNextSample(worker->sampler)) != NULL) {
    struct Color4 C_trace = {0, 0, 0, 0};
    double t_hit = FLT_MAX;
    int hit = 0;

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

    PrgIncrement(worker->progress);
  }
}

int render_scene__(struct Renderer *renderer)
{
	struct FrameBuffer *fb = renderer->framebuffers;
	struct Timer timer;
	struct Elapse elapse;

	/* aux */
	struct Tiler *tiler = NULL;
	struct Tile *tile = NULL;

	int region[4] = {0};
	int render_state = 0;
	int err = 0;

	const int xres = renderer->resolution[0];
	const int yres = renderer->resolution[1];
	const int xtilesize = renderer->tilesize[0];
	const int ytilesize = renderer->tilesize[1];

  struct Worker worker[1];
  init_worker(&worker[0], renderer);

	/* Tiler */
	tiler = TlrNew(xres, yres, xtilesize, ytilesize);
	if (tiler == NULL) {
		render_state = -1;
		goto cleanup_and_exit;
	}

	/* region */
	BOX2_COPY(region, renderer->render_region);
	TlrGenerateTiles(tiler, region[0], region[1], region[2], region[3]);

	/* preprocessing lights */
	printf("Preprocessing lights ...\n");
	TimerStart(&timer);
	err = preprocess_lights(renderer);
	if (err) {
		render_state = -1;
		goto cleanup_and_exit;
	}
	elapse = TimerGetElapse(&timer);
	printf("Done: %dh %dm %gs\n", elapse.hour, elapse.min, elapse.sec);

	/* Run sampling */
	TimerStart(&timer);
	printf("Rendering ...\n");
	while ((tile = TlrGetNextTile(tiler)) != NULL) {
		int pixel_bounds[4] = {0};
		pixel_bounds[0] = tile->xmin;
		pixel_bounds[1] = tile->ymin;
		pixel_bounds[2] = tile->xmax;
		pixel_bounds[3] = tile->ymax;
		if (SmpGenerateSamples(worker->sampler, pixel_bounds)) {
			render_state = -1;
			goto cleanup_and_exit;
		}

    set_working_region(&worker[0], pixel_bounds);

    work_start(&worker[0]);

    integrate_samples(&worker[0]);

    reconstruct_image__(&worker[0], fb);

		printf(" Tile Done: %d/%d (%d %%)\n",
				tile->id+1,
				TlrGetTileCount(tiler),
				(int) ((tile->id+1) / (double) TlrGetTileCount(tiler) * 100));

    work_doen(&worker[0]);
	}
	elapse = TimerGetElapse(&timer);
	printf("Done: %dh %dm %gs\n", elapse.hour, elapse.min, elapse.sec);
	printf("*** render_scene__ ***\n");

cleanup_and_exit:
	TlrFree(tiler);

  finish_worker(&worker[0]);

	return render_state;
}
