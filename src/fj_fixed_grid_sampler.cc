// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_fixed_grid_sampler.h"
#include "fj_rectangle.h"
#include "fj_numeric.h"

namespace fj {

FixedGridSampler::FixedGridSampler() :
  current_index_(0)
{
}

FixedGridSampler::~FixedGridSampler()
{
}

#if 0
int FixedGridSampler::generate_samples(const Rectangle &region)
{
  // allocate samples in region
  nsamples_ = count_samples_in_region(region);
  samples_.resize(nsamples_[0] * nsamples_[1]);
  pixel_start_ = region.min;
  current_index_ = 0;

  XorShift rng; // random number generator
  XorShift rng_time; // for time sampling jitter

  // uv delta
  const Real udelta = 1./(rate_[0] * res_[0] + 2 * margin_[0]);
  const Real vdelta = 1./(rate_[1] * res_[1] + 2 * margin_[1]);

  // xy offset
  const int xoffset = pixel_start_[0] * rate_[0] - margin_[0];
  const int yoffset = pixel_start_[1] * rate_[1] - margin_[1];

  Sample *sample = &samples_[0];

  for (int y = 0; y < nsamples_[1]; y++) {
    for (int x = 0; x < nsamples_[0]; x++) {
      sample->uv.x =     (.5 + x + xoffset) * udelta;
      sample->uv.y = 1 - (.5 + y + yoffset) * vdelta;

      if (need_jitter_) {
        const Real u_jitter = XorNextFloat01(&rng) * jitter_;
        const Real v_jitter = XorNextFloat01(&rng) * jitter_;

        sample->uv.x += udelta * (u_jitter - .5);
        sample->uv.y += vdelta * (v_jitter - .5);
      }

      if (need_time_sampling_) {
        const Real rnd = XorNextFloat01(&rng_time);
        sample->time = Fit(rnd, 0, 1, sample_time_start_, sample_time_end_);
      } else {
        sample->time = 0;
      }

      sample->data = Vector4();
      sample++;
    }
  }
  return 0;
}

Sample *FixedGridSampler::get_next_sample()
{
  if (current_index_ >= GetSampleCount())
    return NULL;

  Sample *sample = &samples_[current_index_];
  current_index_++;

  return sample;
}
#endif

Int2 FixedGridSampler::count_samples_in_margin() const
{
  return Int2(
      static_cast<int>(Ceil(((GetFilterWidth()[0] - 1) * GetPixelSamples()[0]) * .5)),
      static_cast<int>(Ceil(((GetFilterWidth()[1] - 1) * GetPixelSamples()[1]) * .5)));
}

Int2 FixedGridSampler::count_samples_in_pixel() const
{
  return  GetPixelSamples() + 2 * GetMargin();
}

Int2 FixedGridSampler::count_samples_in_region(const Rectangle &region) const
{
  return GetPixelSamples() * region.Size() + 2 * GetMargin();
}

} // namespace xxx
