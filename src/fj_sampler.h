// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SAMPLER_H
#define FJ_SAMPLER_H

#include "fj_pixel_sample.h"
#include "fj_vector.h"
#include "fj_types.h"
#include <vector>

namespace fj {

class Rectangle;

class Sampler {
public:
  Sampler();
  ~Sampler();

  void Initialize(int xres, int yres,
      int xsamples, int ysamples, float xfwidth, float yfwidth);

  void SetJitter(Real jitter);
  void SetSampleTimeRange(Real start_time, Real end_time);

  // interfaces for a region
  int GenerateSamples(const Rectangle &pixel_bounds);
  int GetSampleCount() const;
  Sample *GetNextSample();
  int ComputeSampleCountForRegion(const Rectangle &region) const;

  // interfaces for a pixel
  int GetSampleCountForPixel() const;
  void GetSampleSetForPixel(std::vector<Sample> &pixelsamples,
      int pixel_x, int pixel_y) const;

private:
  void count_samples_in_pixels();
  int allocate_samples_for_region(const Rectangle &region);

  Int2 res_;
  Int2 rate_;
  Vector2 fwidth_;
  Real jitter_;
  std::vector<Sample> samples_;

  Int2 nsamples_;
  Int2 pixel_start_;
  Int2 margin_;
  Int2 npxlsmps_;

  int current_index_;

  int need_jitter_;
  int need_time_sampling_;

  Real sample_time_start_;
  Real sample_time_end_;
};

} // namespace xxx

#endif // FJ_XXX_H
