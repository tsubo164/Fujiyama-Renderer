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
/*
  virtual int generate_samples(const Rectangle &region);
  virtual Sample *get_next_sample();
*/
  virtual Int2 count_samples_in_margin() const;
  virtual Int2 count_samples_in_pixel() const;
  virtual Int2 count_samples_in_region(const Rectangle &region) const;

  int current_index_;
};

} // namespace xxx

#endif // FJ_XXX_H
