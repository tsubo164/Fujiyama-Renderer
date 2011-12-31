/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Sampler.h"
#include "Box.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

struct Sampler {
	int xres, yres;
	int xrate, yrate;
	float xfwidth, yfwidth;
	struct Sample *samples;

	int xnsamples, ynsamples;
	int xpixel_start, ypixel_start;
	int xmargin, ymargin;
	int xnpxlsmps, ynpxlsmps;

	int nsamples;
	int current_index;
};

static void compute_margins(struct Sampler *sampler);
static int allocate_all_samples(struct Sampler *sampler);

struct Sampler *SmpNew(int xres, int yres,
		int xsamples, int ysamples, float xfwidth, float yfwidth)
{
	struct Sampler *sampler;

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

struct Sample *SmpGetNextSample(struct Sampler *sampler)
{
	struct Sample *sample;

	if (sampler->current_index >= SmpGetSampleCount(sampler))
		return NULL;

	sample = sampler->samples + sampler->current_index;
	sampler->current_index++;

	return sample;
}

int SmpGenerateSamples(struct Sampler *sampler, const int *pixel_bounds)
{
	int x, y;
	double udelta;
	double vdelta;
	struct Sample *smp;

	/* region related info */
	sampler->xnsamples = sampler->xrate * BOX2_XSIZE(pixel_bounds) + 2 * sampler->xmargin;
	sampler->ynsamples = sampler->yrate * BOX2_YSIZE(pixel_bounds) + 2 * sampler->ymargin;
	sampler->xpixel_start = pixel_bounds[0];
	sampler->ypixel_start = pixel_bounds[1];

	/* uv delta */
	udelta = 1./(sampler->xrate * sampler->xres + 2 * sampler->xmargin);
	vdelta = 1./(sampler->yrate * sampler->yres + 2 * sampler->ymargin);

	if (allocate_all_samples(sampler))
		return -1;

	smp = sampler->samples;
	for (y = 0; y < sampler->ynsamples; y++) {
		for (x = 0; x < sampler->xnsamples; x++) {
			smp->uv[0] = (.5 + x + sampler->xpixel_start * sampler->xrate - sampler->xmargin) * udelta;
			smp->uv[1] = 1 - (.5 + y + sampler->ypixel_start * sampler->yrate - sampler->xmargin) * vdelta;

			smp->data[0] = 0;
			smp->data[1] = 0;
			smp->data[2] = 0;
			smp->data[3] = 0;
			smp++;
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
	struct Sample *samples;

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

static int allocate_all_samples(struct Sampler *sampler)
{
	int old_nsamples;

	old_nsamples = SmpGetSampleCount(sampler);
	sampler->nsamples = sampler->xnsamples * sampler->ynsamples;

	if (old_nsamples < SmpGetSampleCount(sampler)) {
		sampler->samples = (struct Sample *) malloc(sizeof(struct Sample) *
				SmpGetSampleCount(sampler));
		if (sampler->samples == NULL) {
			return -1;
		}
	}
	sampler->current_index = 0;

	return 0;
}

