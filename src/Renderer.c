/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Renderer.h"
#include "FrameBuffer.h"
#include "Progress.h"
#include "Property.h"
#include "Numeric.h"
#include "Sampler.h"
#include "Camera.h"
#include "Filter.h"
#include "Vector.h"
#include "Light.h"
#include "Tiler.h"
#include "Timer.h"
#include "Ray.h"
#include "Box.h"
#include "SL.h"

#include <assert.h>
#include <stdlib.h>
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

struct Renderer *RdrNew(void)
{
	struct Renderer *renderer = (struct Renderer *) malloc(sizeof(struct Renderer));

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
	free(renderer);
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

	err = render_scene(renderer);
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

	int x, y;
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
	elapse = TimerElapsed(&timer);
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
			float C_trace[4] = {0};
			double t_hit = FLT_MAX;
			int hit = 0;

			CamGetRay(cam, smp->uv, smp->time, &ray);
			cxt.time = smp->time;

			hit = SlTrace(&cxt, ray.orig, ray.dir, ray.tmin, ray.tmax, C_trace, &t_hit);
			VEC4_COPY(smp->data, C_trace);

			PrgIncrement(progress);
		}

		for (y = pixel_bounds[1]; y < pixel_bounds[3]; y++) {
			for (x = pixel_bounds[0]; x < pixel_bounds[2]; x++) {
				const int NSAMPLES = SmpGetSampleCountForPixel(sampler);
				float *dst = NULL;
				float pixel[4] = {0};
				float sum = 0;
				int i;

				SmpGetPixelSamples(sampler, pixelsmps, x, y);

				for (i = 0; i < NSAMPLES; i++) {
					struct Sample *sample = pixelsmps + i;
					double filtx, filty;
					double wgt;

					filtx = xres * sample->uv[0] - (x+.5);
					filty = yres * (1-sample->uv[1]) - (y+.5);
					wgt = FltEvaluate(filter, filtx, filty);
					pixel[0] += wgt * sample->data[0];
					pixel[1] += wgt * sample->data[1];
					pixel[2] += wgt * sample->data[2];
					pixel[3] += wgt * sample->data[3];
					sum += wgt;
				}
				VEC4_DIV(pixel, sum);

				dst = FbGetWritable(fb, x, y, 0);
				VEC4_COPY(dst, pixel);
			}
		}
		printf(" Tile Done: %d/%d (%d %%)\n",
				tile->id+1,
				TlrGetTileCount(tiler),
				(int ) ((tile->id+1) / (double )TlrGetTileCount(tiler) * 100));
		PrgDone(progress);
	}
	elapse = TimerElapsed(&timer);
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

