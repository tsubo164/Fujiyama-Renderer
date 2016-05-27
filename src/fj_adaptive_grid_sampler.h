// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_ADAPTIVE_GRID_SAMPLER_H
#define FJ_ADAPTIVE_GRID_SAMPLER_H

#include "fj_sampler.h"
#include <vector>
#include <stack>

namespace fj {

class AdaptiveGridSampler : public Sampler {
public:
  AdaptiveGridSampler();
  virtual ~AdaptiveGridSampler();

private:
  virtual void update_sample_counts();
  virtual int generate_samples(const Rectangle &region);
  virtual Sample *get_next_sample();
  virtual void get_sampleset_in_pixel(std::vector<Sample> &pixelsamples,
      const Int2 &pixel_pos) const;

  int get_sample_count() const;
  Int2 count_samples_in_region(const Rectangle &region) const;
  Int2 count_samples_in_pixel() const;
  Int2 count_samples_in_margin() const;

  Int2 compute_num_pixel_division() const;
  bool compare_corners(const Rectangle rect);
  void subdivide_rect(const Rectangle rect);
  void interpolate_rect(const Rectangle rect);
  bool subdivide_or_interpolate(const Rectangle rect);

  std::vector<Sample> samples_;

  Int2 nsamples_;
  Int2 pixel_start_;
  Int2 margin_;
  Int2 npxlsmps_;
  Int2 ndivision_;

  typedef std::stack<Rectangle,std::vector<Rectangle> > Stack;
  Stack subd_stack_;
  int  curr_corner_;

  std::vector<float> subd_flag_;
};

} // namespace xxx

#endif // FJ_XXX_H
