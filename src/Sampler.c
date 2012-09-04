/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Sampler.h"
#include "Random.h"
#include "Box.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

struct Sampler {
	int xres, yres;
	int xrate, yrate;
	float xfwidth, yfwidth;
	float jitter;
	struct Sample *samples;

	int xnsamples, ynsamples;
	int xpixel_start, ypixel_start;
	int xmargin, ymargin;
	int xnpxlsmps, ynpxlsmps;

	int nsamples;
	int current_index;

	int need_jitter;
	int need_time_sampling;
};

static void compute_margins(struct Sampler *sampler);
static int allocate_samples_for_region(struct Sampler *sampler, const int *region);

struct Sampler *SmpNew(int xres, int yres,
		int xsamples, int ysamples, float xfwidth, float yfwidth)
{
	struct Sampler *sampler = NULL;

	sampler = (struct Sampler *) malloc(sizeof(struct Sampler));
	if (sampler == NULL)
		return NULL;

	assert(xres > 0);
	assert(yres > 0);
	assert(xsamples > 0);
	assert(ysamples > 0);
	assert(xfwidth > 0);
	assert(yfwidth > 0);

	sampler->xres = xres;
	sampler->yres = yres;
	sampler->xrate = xsamples;
	sampler->yrate = ysamples;
	sampler->xfwidth = xfwidth;
	sampler->yfwidth = yfwidth;
	SmpSetJitter(sampler, 1);
	SmpSetTimeSampling(sampler);
	sampler->samples = NULL;

	sampler->nsamples = 0;
	sampler->current_index = 0;

	compute_margins(sampler);
	return sampler;
}

void SmpFree(struct Sampler *sampler)
{
	if (sampler == NULL)
		return;

	if (sampler->samples != NULL)
		free(sampler->samples);

	free(sampler);
}

void SmpSetJitter(struct Sampler *sampler, float jitter)
{
	assert(jitter >= 0 && jitter <= 1);

	sampler->jitter = jitter;
	sampler->need_jitter = sampler->jitter > 0 ? 1 : 0;
}

void SmpSetTimeSampling(struct Sampler *sampler)
{
	sampler->need_time_sampling = 1;
}

struct Sample *SmpGetNextSample(struct Sampler *sampler)
{
	struct Sample *sample = NULL;

	if (sampler->current_index >= SmpGetSampleCount(sampler))
		return NULL;

	sample = sampler->samples + sampler->current_index;
	sampler->current_index++;

	return sample;
}

/* TODO should match tiler interface e.g. xmin, ymin ... */
int SmpGenerateSamples(struct Sampler *sampler, const int *pixel_bounds)
{
	int x, y;
	int err = 0;
	int xoffset = 0;
	int yoffset = 0;
	double udelta = 0;
	double vdelta = 0;
	struct Sample *sample = NULL;
	struct XorShift xr; /* random number generator */
	struct XorShift rng_time; /* for time sampling jitter */
	XorInit(&rng_time);
	XorInit(&xr);

	err = allocate_samples_for_region(sampler, pixel_bounds);
	if (err) {
		return -1;
	}

	/* uv delta */
	udelta = 1./(sampler->xrate * sampler->xres + 2 * sampler->xmargin);
	vdelta = 1./(sampler->yrate * sampler->yres + 2 * sampler->ymargin);

	/* xy offset */
	xoffset = sampler->xpixel_start * sampler->xrate - sampler->xmargin;
	yoffset = sampler->ypixel_start * sampler->yrate - sampler->ymargin;

	sample = sampler->samples;
	for (y = 0; y < sampler->ynsamples; y++) {
		for (x = 0; x < sampler->xnsamples; x++) {
			sample->uv[0] =     (.5 + x + xoffset) * udelta;
			sample->uv[1] = 1 - (.5 + y + yoffset) * vdelta;

			if (sampler->need_jitter) {
				const double u_jitter = XorNextFloat01(&xr) * sampler->jitter;
				const double v_jitter = XorNextFloat01(&xr) * sampler->jitter;

				sample->uv[0] += udelta * (u_jitter - .5);
				sample->uv[1] += vdelta * (v_jitter - .5);
			}

			if (sampler->need_time_sampling) {
				sample->time = XorNextFloat01(&rng_time);
			} else {
				sample->time = 0;
			}

			sample->data[0] = 0;
			sample->data[1] = 0;
			sample->data[2] = 0;
			sample->data[3] = 0;
			sample++;
		}
	}
	return 0;
}

int SmpGetSampleCount(const struct Sampler *sampler)
{
	return sampler->nsamples;
}

void SmpGetPixelSamples(struct Sampler *sampler, struct Sample *pixelsamples,
		int pixel_x, int pixel_y)
{
	int x, y;
	const int XPIXEL_OFFSET = pixel_x - sampler->xpixel_start;
	const int YPIXEL_OFFSET = pixel_y - sampler->ypixel_start;

	const int XNSAMPLES = sampler->xnsamples;
	const int OFFSET =
		YPIXEL_OFFSET * sampler->yrate * XNSAMPLES +
		XPIXEL_OFFSET * sampler->xrate;
	struct Sample *src = sampler->samples + OFFSET;
	struct Sample *dst = pixelsamples;

	for (y = 0; y < sampler->ynpxlsmps; y++) {
		for (x = 0; x < sampler->xnpxlsmps; x++) {
			dst[y * sampler->xnpxlsmps + x] = src[y * XNSAMPLES + x];
		}
	}
}

struct Sample *SmpAllocatePixelSamples(struct Sampler *sampler)
{
	struct Sample *samples = NULL;

	samples = (struct Sample *) malloc(sizeof(struct Sample) *
			SmpGetSampleCountForPixel(sampler));

	return samples;
}

int SmpGetSampleCountForPixel(const struct Sampler *sampler)
{
	return sampler->xnpxlsmps * sampler->ynpxlsmps;
}

void SmpFreePixelSamples(struct Sample *samples)
{
	if (samples == NULL)
		return;
	free(samples);
}

static void compute_margins(struct Sampler *sampler)
{
	sampler->xmargin = (int) ceil(((sampler->xfwidth-1) * sampler->xrate) * .5);
	sampler->ymargin = (int) ceil(((sampler->yfwidth-1) * sampler->yrate) * .5);
	sampler->xnpxlsmps = sampler->xrate + 2 * sampler->xmargin;
	sampler->ynpxlsmps = sampler->yrate + 2 * sampler->ymargin;
}

static int allocate_samples_for_region(struct Sampler *sampler, const int *region)
{
	const int XNSAMPLES = sampler->xrate * BOX2_XSIZE(region) + 2 * sampler->xmargin;
	const int YNSAMPLES = sampler->yrate * BOX2_YSIZE(region) + 2 * sampler->ymargin;
	const int XPIXEL_START = region[0];
	const int YPIXEL_START = region[1];

	const int OLD_NSAMPLES = SmpGetSampleCount(sampler);
	const int NEW_NSAMPLES = XNSAMPLES * YNSAMPLES;

	if (OLD_NSAMPLES < NEW_NSAMPLES) {
		sampler->samples = (struct Sample *) malloc(sizeof(struct Sample) * NEW_NSAMPLES);
		if (sampler->samples == NULL) {
			return -1;
		}
	}

	sampler->xnsamples = XNSAMPLES;
	sampler->ynsamples = YNSAMPLES;
	sampler->xpixel_start = XPIXEL_START;
	sampler->ypixel_start = YPIXEL_START;

	sampler->nsamples = NEW_NSAMPLES;
	sampler->current_index = 0;

	return 0;
}

