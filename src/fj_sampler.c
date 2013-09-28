/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_sampler.h"
#include "fj_rectangle.h"
#include "fj_numeric.h"
#include "fj_memory.h"
#include "fj_random.h"
#include "fj_array.h"
#include "fj_box.h"

#include <assert.h>
#include <math.h>

struct Sampler {
  int xres, yres;
  int xrate, yrate;
  float xfwidth, yfwidth;
  float jitter;
  struct Array *samples;

  int xnsamples, ynsamples;
  int xpixel_start, ypixel_start;
  int xmargin, ymargin;
  int xnpxlsmps, ynpxlsmps;

  int current_index;

  int need_jitter;
  int need_time_sampling;

  double sample_time_start;
  double sample_time_end;
};

#define SAMPLE_ARRAY(sampler) ((struct Sample *)(sampler)->samples->data)

static void compute_margins(struct Sampler *sampler);
static int allocate_samples_for_region(struct Sampler *sampler, const struct Rectangle *region);

struct Sampler *SmpNew(int xres, int yres,
    int xsamples, int ysamples, float xfwidth, float yfwidth)
{
  struct Sampler *sampler = SI_MEM_ALLOC(struct Sampler);

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
  SmpSetSampleTimeRange(sampler, 0, 1);
  sampler->samples = ArrNew(sizeof(struct Sample));

  sampler->current_index = 0;

  compute_margins(sampler);
  return sampler;
}

void SmpFree(struct Sampler *sampler)
{
  if (sampler == NULL)
    return;
  ArrFree(sampler->samples);
  SI_MEM_FREE(sampler);
}

void SmpSetJitter(struct Sampler *sampler, float jitter)
{
  assert(jitter >= 0 && jitter <= 1);

  sampler->jitter = jitter;
  sampler->need_jitter = sampler->jitter > 0 ? 1 : 0;
}

void SmpSetSampleTimeRange(struct Sampler *sampler, double start_time, double end_time)
{
  assert(start_time <= end_time);

  sampler->sample_time_start = start_time;
  sampler->sample_time_end = end_time;

  /* TODO need this member? */
  sampler->need_time_sampling = 1;
}

struct Sample *SmpGetNextSample(struct Sampler *sampler)
{
  struct Sample *sample = NULL;

  if (sampler->current_index >= SmpGetSampleCount(sampler))
    return NULL;

  sample = SAMPLE_ARRAY(sampler) + sampler->current_index;
  sampler->current_index++;

  return sample;
}

int SmpGenerateSamples(struct Sampler *sampler, const struct Rectangle *pixel_bounds)
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

  sample = SAMPLE_ARRAY(sampler);
  for (y = 0; y < sampler->ynsamples; y++) {
    for (x = 0; x < sampler->xnsamples; x++) {
      sample->uv.x =     (.5 + x + xoffset) * udelta;
      sample->uv.y = 1 - (.5 + y + yoffset) * vdelta;

      if (sampler->need_jitter) {
        const double u_jitter = XorNextFloat01(&xr) * sampler->jitter;
        const double v_jitter = XorNextFloat01(&xr) * sampler->jitter;

        sample->uv.x += udelta * (u_jitter - .5);
        sample->uv.y += vdelta * (v_jitter - .5);
      }

      if (sampler->need_time_sampling) {
        sample->time = XorNextFloat01(&rng_time);
        sample->time = Fit(sample->time,
            0,
            1,
            sampler->sample_time_start,
            sampler->sample_time_end);
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
  return sampler->samples->nelems;
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
  struct Sample *src = SAMPLE_ARRAY(sampler) + OFFSET;
  struct Sample *dst = pixelsamples;

  for (y = 0; y < sampler->ynpxlsmps; y++) {
    for (x = 0; x < sampler->xnpxlsmps; x++) {
      dst[y * sampler->xnpxlsmps + x] = src[y * XNSAMPLES + x];
    }
  }
}

struct Sample *SmpAllocatePixelSamples(struct Sampler *sampler)
{
  const int sample_count = SmpGetSampleCountForPixel(sampler);
  struct Sample *samples = SI_MEM_ALLOC_ARRAY(struct Sample, sample_count);

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
  SI_MEM_FREE(samples);
}

static void compute_margins(struct Sampler *sampler)
{
  sampler->xmargin = (int) ceil(((sampler->xfwidth-1) * sampler->xrate) * .5);
  sampler->ymargin = (int) ceil(((sampler->yfwidth-1) * sampler->yrate) * .5);
  sampler->xnpxlsmps = sampler->xrate + 2 * sampler->xmargin;
  sampler->ynpxlsmps = sampler->yrate + 2 * sampler->ymargin;
}

#define SIZE_X(rect) ((rect)->xmax-(rect)->xmin)
#define SIZE_Y(rect) ((rect)->ymax-(rect)->ymin)

static int allocate_samples_for_region(struct Sampler *sampler, const struct Rectangle *region)
{
  const int XNSAMPLES = sampler->xrate * SIZE_X(region) + 2 * sampler->xmargin;
  const int YNSAMPLES = sampler->yrate * SIZE_Y(region) + 2 * sampler->ymargin;
  const int NEW_NSAMPLES = XNSAMPLES * YNSAMPLES;
  const int XPIXEL_START = region->xmin;
  const int YPIXEL_START = region->ymin;

  ArrResize(sampler->samples, NEW_NSAMPLES);

  sampler->xnsamples = XNSAMPLES;
  sampler->ynsamples = YNSAMPLES;
  sampler->xpixel_start = XPIXEL_START;
  sampler->ypixel_start = YPIXEL_START;

  sampler->current_index = 0;

  return 0;
}
