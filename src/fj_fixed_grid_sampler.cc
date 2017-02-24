// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_fixed_grid_sampler.h"
#include "fj_rectangle.h"
#include "fj_numeric.h"
#include "fj_random.h"

namespace fj {

FixedGridSampler::FixedGridSampler() :
  samples_(),

  nsamples_(1, 1),
  pixel_start_(0, 0),
  margin_(0, 0),
  npxlsmps_(1, 1),

  current_index_(0)
{
}

FixedGridSampler::~FixedGridSampler()
{
}

void FixedGridSampler::update_sample_counts()
{
  margin_ = count_samples_in_margin();
  npxlsmps_ = count_samples_in_pixel();
}

int FixedGridSampler::generate_samples(const Rectangle &region)
{
  // allocate samples in region
  nsamples_ = count_samples_in_region(region);
  samples_.resize(nsamples_[0] * nsamples_[1]);
  pixel_start_ = region.min;
  current_index_ = 0;

  XorShift rng; // random number generator
  XorShift rng_time; // for time sampling jitter

  const Int2 rate = GetPixelSamples();
  const Int2 res  = GetResolution();
  const Real jitter = GetJitter();
  const Vector2 sample_time_range = GetSampleTimeRange();

  // uv delta (screen space uv. excludes margins)
  const Real udelta = 1./(rate[0] * res[0]);
  const Real vdelta = 1./(rate[1] * res[1]);

  // xy offset
  const int xoffset = pixel_start_[0] * rate[0] - margin_[0];
  const int yoffset = pixel_start_[1] * rate[1] - margin_[1];

  Sample *sample = &samples_[0];

  for (int y = 0; y < nsamples_[1]; y++) {
    for (int x = 0; x < nsamples_[0]; x++) {
      sample->uv.x =     (.5 + x + xoffset) * udelta;
      sample->uv.y = 1 - (.5 + y + yoffset) * vdelta;

      if (IsJittered()) {
        const Real u_jitter = rng.NextFloat01() * jitter;
        const Real v_jitter = rng.NextFloat01() * jitter;

        sample->uv.x += udelta * (u_jitter - .5);
        sample->uv.y += vdelta * (v_jitter - .5);
      }

      if (IsSamplingTime()) {
        const Real rnd = rng_time.NextFloat01();
        sample->time = Fit(rnd, 0, 1, sample_time_range[0], sample_time_range[1]);
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
  if (current_index_ >= get_sample_count())
    return NULL;

  Sample *sample = &samples_[current_index_];
  current_index_++;

  return sample;
}

void FixedGridSampler::get_sampleset_in_pixel(std::vector<Sample> &pixelsamples,
    const Int2 &pixel_pos) const
{
  const Int2 rate = GetPixelSamples();

  const int XPIXEL_OFFSET = pixel_pos[0] - pixel_start_[0];
  const int YPIXEL_OFFSET = pixel_pos[1] - pixel_start_[1];

  const int XNSAMPLES = nsamples_[0];
  const int OFFSET =
    YPIXEL_OFFSET * rate[1] * XNSAMPLES +
    XPIXEL_OFFSET * rate[0];
  const Sample *src = &samples_[OFFSET];

  const Int2 NPXLSMPS = count_samples_in_pixel();
  const std::size_t SAMPLE_COUNT = static_cast<std::size_t>(NPXLSMPS[0] * NPXLSMPS[1]);
  if (pixelsamples.size() < SAMPLE_COUNT) {
    pixelsamples.resize(SAMPLE_COUNT);
  }

  std::vector<Sample> &dst = pixelsamples;

  for (int y = 0; y < npxlsmps_[1]; y++) {
    for (int x = 0; x < npxlsmps_[0]; x++) {
      dst[y * npxlsmps_[0] + x] = src[y * XNSAMPLES + x];
    }
  }
}

int FixedGridSampler::get_sample_count() const
{
  return samples_.size();
}

Int2 FixedGridSampler::count_samples_in_margin() const
{
  return Int2(
      static_cast<int>(Ceil(((GetFilterWidth()[0] - 1) * GetPixelSamples()[0]) * .5)),
      static_cast<int>(Ceil(((GetFilterWidth()[1] - 1) * GetPixelSamples()[1]) * .5)));
}

Int2 FixedGridSampler::count_samples_in_pixel() const
{
  return  GetPixelSamples() + 2 * margin_;
}

Int2 FixedGridSampler::count_samples_in_region(const Rectangle &region) const
{
  return GetPixelSamples() * region.Size() + 2 * margin_;
}

} // namespace xxx
