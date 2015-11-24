// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_FIXED_GRID_SAMPLER_H
#define FJ_FIXED_GRID_SAMPLER_H

#include "fj_sampler.h"

namespace fj {

class FixedGridSampler : public Sampler {
public:
  FixedGridSampler();
  virtual ~FixedGridSampler();

private:
  virtual int generate_samples(const Rectangle &region);
  virtual Sample *get_next_sample();
  virtual void get_sampleset_in_pixel(std::vector<Sample> &pixelsamples,
      const Int2 &pixel_pos) const;
  virtual int get_sample_count() const;
  virtual Int2 count_samples_in_pixel() const;
  virtual Int2 count_samples_in_region(const Rectangle &region) const;
  virtual void update_sample_counts();

  Int2 count_samples_in_margin() const;

  std::vector<Sample> samples_;

  Int2 nsamples_;
  Int2 pixel_start_;
  Int2 margin_;
  Int2 npxlsmps_;

  int current_index_;
};

} // namespace xxx

#endif // FJ_XXX_H
