// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_sampler.h"
#include "fj_rectangle.h"
#include "fj_numeric.h"
#include "fj_random.h"
#include "fj_box.h"

#include <cstddef>
#include <cassert>
#include <cmath>

namespace fj {

Sampler::Sampler() :
  res_(1, 1),
  rate_(1, 1),
  fwidth_(1., 1.),
  jitter_(1.),

  samples_(),

  nsamples_(1, 1),
  pixel_start_(0, 0),
  margin_(0, 0),
  npxlsmps_(1, 1),

  current_index_(0),

  need_jitter_(true),
  need_time_sampling_(false),

  sample_time_start_(0),
  sample_time_end_(0)
{
}

Sampler::~Sampler()
{
}

void Sampler::SetResolution(const Int2 &resolution)
{
  assert(resolution[0] > 0 && resolution[1] > 0);
  res_ = resolution;
  update_sample_counts();
}

void Sampler::SetPixelSamples(const Int2 &pixel_samples)
{
  assert(pixel_samples[0] > 0 && pixel_samples[1] > 0);
  rate_ = pixel_samples;
  update_sample_counts();
}

void Sampler::SetFilterWidth(const Vector2 &filter_width)
{
  assert(filter_width[0] > 0 && filter_width[1] > 0);
  fwidth_ = filter_width;
  update_sample_counts();
}

void Sampler::SetJitter(Real jitter)
{
  assert(jitter >= 0 && jitter <= 1);

  jitter_ = jitter;
  need_jitter_ = jitter_ > 0 ? 1 : 0;
}

void Sampler::SetSampleTimeRange(Real start_time, Real end_time)
{
  assert(start_time <= end_time);

  sample_time_start_ = start_time;
  sample_time_end_ = end_time;

  // TODO need this member?
  need_time_sampling_ = true;
}

const Int2 &Sampler::SetResolution() const
{
  return res_;
}

const Int2 &Sampler::GetPixelSamples() const
{
  return rate_;
}

const Vector2 &Sampler::GetFilterWidth() const
{
  return fwidth_;
}

const Int2 &Sampler::GetMargin() const
{
  return margin_;
}

int Sampler::GenerateSamples(const Rectangle &region)
{
  return generate_samples(region);
}

int Sampler::GetSampleCount() const
{
  return samples_.size();
}

Sample *Sampler::GetNextSample()
{
  return get_next_sample();
}

void Sampler::GetSampleSetInPixel(std::vector<Sample> &pixelsamples,
    int pixel_x, int pixel_y) const
{
  const int XPIXEL_OFFSET = pixel_x - pixel_start_[0];
  const int YPIXEL_OFFSET = pixel_y - pixel_start_[1];

  const int XNSAMPLES = nsamples_[0];
  const int OFFSET =
    YPIXEL_OFFSET * rate_[1] * XNSAMPLES +
    XPIXEL_OFFSET * rate_[0];
  const Sample *src = &samples_[OFFSET];

  const std::size_t SAMPLE_COUNT = static_cast<std::size_t>(GetSampleCountInPixel());
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

int Sampler::GetSampleCountInPixel() const
{
  return npxlsmps_[0] * npxlsmps_[1];
}

int Sampler::ComputeSampleCountInRegion(const Rectangle &region) const
{
  const Int2 nsamples = count_samples_in_region(region);
  return nsamples[0] * nsamples[1];
}

void Sampler::update_sample_counts()
{
  margin_ = count_samples_in_margin();
  npxlsmps_ = count_samples_in_pixel();
}

int Sampler::generate_samples(const Rectangle &region)
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

Sample *Sampler::get_next_sample()
{
  if (current_index_ >= GetSampleCount())
    return NULL;

  Sample *sample = &samples_[current_index_];
  current_index_++;

  return sample;
}
/*
*/

} // namespace xxx
