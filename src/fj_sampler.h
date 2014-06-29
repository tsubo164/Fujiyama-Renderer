// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SAMPLER_H
#define FJ_SAMPLER_H

#include "fj_vector.h"
#include "fj_types.h"
#include <vector>

namespace fj {

class Rectangle;

class Sample {
public:
  Sample() : uv(), time(0), data() {}
  ~Sample() {}

public:
  Vector2 uv;
  Real time;
  Vector4 data;
};

class Sampler {
public:
  Sampler();
  ~Sampler();

  void Initialize(int xres, int yres,
      int xsamples, int ysamples, float xfwidth, float yfwidth);

  void SetJitter(float jitter);
  void SetSampleTimeRange(Real start_time, Real end_time);

  // interfaces for a region
  int GenerateSamples(const Rectangle &pixel_bounds);
  int GetSampleCount() const;
  Sample *GetNextSample();
  void GetPixelSamples(Sample *pixelsamples, int pixel_x, int pixel_y) const;

  // interfaces for a pixel
  Sample *AllocatePixelSamples();
  int GetSampleCountForPixel() const;
  void FreePixelSamples(Sample *samples) const;

  static int GetSampleCountForRegion(const Rectangle &region,
      int xrate, int yrate, float xfwidth, float yfwidth);

private:
  void count_samples_in_pixels();
  int allocate_samples_for_region(const Rectangle &region);

  int xres_, yres_;
  int xrate_, yrate_;
  float xfwidth_, yfwidth_;
  float jitter_;
  std::vector<Sample> samples_;

  int xnsamples_, ynsamples_;
  int xpixel_start_, ypixel_start_;
  int xmargin_, ymargin_;
  int xnpxlsmps_, ynpxlsmps_;

  int current_index_;

  int need_jitter_;
  int need_time_sampling_;

  Real sample_time_start_;
  Real sample_time_end_;
};

} // namespace xxx

#endif // FJ_XXX_H
