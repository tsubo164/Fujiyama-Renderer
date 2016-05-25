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

  // TODO ADAPTIVE_TEST
  Int2 compute_num_pixel_division() const;
  bool need_subd_rect(const Rectangle rect);

  std::vector<Sample> samples_;

  Int2 nsamples_;
  Int2 pixel_start_;
  Int2 margin_;
  Int2 npxlsmps_;
  Int2 ndivision_;

  int current_index_;

  // TODO ADAPTIVE_TEST
  typedef std::stack<Rectangle,std::vector<Rectangle> > Stack;
  Stack rect_stack_;
  Int2 curr_pixel_;
  int  curr_corner_;

  std::vector<int> subd_flag_;
};

} // namespace xxx

#endif // FJ_XXX_H
