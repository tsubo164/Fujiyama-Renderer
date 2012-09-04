/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef SAMPLER_H
#define SAMPLER_H

#ifdef __cplusplus
extern "C" {
#endif

struct Sampler;

struct Sample {
	double uv[2];
	double time;

	double data[4];
};

extern struct Sampler *SmpNew(int xres, int yres,
		int xsamples, int ysamples, float xfwidth, float yfwidth);
extern void SmpFree(struct Sampler *sampler);

extern void SmpSetJitter(struct Sampler *sampler, float jitter);
extern void SmpSetTimeSampling(struct Sampler *sampler);

/* interfaces for a region */
extern int SmpGenerateSamples(struct Sampler *sampler, const int *pixel_bounds);
extern int SmpGetSampleCount(const struct Sampler *sampler);
extern struct Sample *SmpGetNextSample(struct Sampler *sampler);
extern void SmpGetPixelSamples(struct Sampler *sampler, struct Sample *pixelsamples,
		int pixel_x, int pixel_y);

/* interfaces for a pixel */
extern struct Sample *SmpAllocatePixelSamples(struct Sampler *sampler);
extern int SmpGetSampleCountForPixel(const struct Sampler *sampler);
extern void SmpFreePixelSamples(struct Sample *samples);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

