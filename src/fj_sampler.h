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

class Int2 {
public:
  Int2()
    : x(0), y(0) {}
  Int2(int xx, int yy)
    : x(xx), y(yy) {}
  Int2(const Int2 &a)
    : x(a[0]), y(a[1]) {}
  ~Int2() {}

  int operator[](int i) const
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    default:
      assert(!"bounds error at Int2::get");
      return x;
    }
  }
  int &operator[](int i)
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    default:
      assert(!"bounds error at Int2::set");
      return x;
    }
  }

  int x, y;
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
  int ComputeSampleCountForRegion(const Rectangle &region) const;

  // interfaces for a pixel
  int GetSampleCountForPixel() const;
  void GetSampleSetForPixel(std::vector<Sample> &pixelsamples,
      int pixel_x, int pixel_y) const;

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
