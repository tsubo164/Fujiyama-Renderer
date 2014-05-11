/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_SAMPLER_H
#define FJ_SAMPLER_H

#include "fj_vector.h"

namespace fj {

struct Sample {
  struct Vector2 uv;
  double time;

  double data[4];
};

struct Sampler;
struct Rectangle;

extern struct Sampler *SmpNew(int xres, int yres,
    int xsamples, int ysamples, float xfwidth, float yfwidth);
extern void SmpFree(struct Sampler *sampler);

extern void SmpSetJitter(struct Sampler *sampler, float jitter);
extern void SmpSetSampleTimeRange(struct Sampler *sampler, double start_time, double end_time);

/* interfaces for a region */
extern int SmpGenerateSamples(struct Sampler *sampler, const struct Rectangle *pixel_bounds);
extern int SmpGetSampleCount(const struct Sampler *sampler);
extern struct Sample *SmpGetNextSample(struct Sampler *sampler);
extern void SmpGetPixelSamples(struct Sampler *sampler, struct Sample *pixelsamples,
    int pixel_x, int pixel_y);

/* interfaces for a pixel */
extern struct Sample *SmpAllocatePixelSamples(struct Sampler *sampler);
extern int SmpGetSampleCountForPixel(const struct Sampler *sampler);
extern void SmpFreePixelSamples(struct Sample *samples);

extern int SmpGetSampleCountForRegion(const struct Rectangle *region,
    int xrate, int yrate, float xfwidth, float yfwidth);

} // namespace xxx

#endif /* FJ_XXX_H */
