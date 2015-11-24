// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_sampler.h"
#include "fj_rectangle.h"
#include "fj_numeric.h"
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

const Int2 &Sampler::GetResolution() const
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

Vector2 Sampler::GetSampleTimeRange() const
{
  return Vector2(sample_time_start_, sample_time_end_);
}

bool Sampler::IsSamplingTime() const
{
  return need_time_sampling_;
}

bool Sampler::IsJittered() const
{
  return need_jitter_;
}

Real Sampler::GetJitter() const
{
  return jitter_;
}

int Sampler::GenerateSamples(const Rectangle &region)
{
  return generate_samples(region);
}

int Sampler::GetSampleCount() const
{
  return get_sample_count();
}

Sample *Sampler::GetNextSample()
{
  return get_next_sample();
}

void Sampler::GetSampleSetInPixel(std::vector<Sample> &pixelsamples,
    int pixel_x, int pixel_y) const
{
  get_sampleset_in_pixel(pixelsamples, Int2(pixel_x, pixel_y));
}

int Sampler::GetSampleCountInPixel() const
{
  Int2 npxlsmps = count_samples_in_pixel();
  return npxlsmps[0] * npxlsmps[1];
}

int Sampler::ComputeSampleCountInRegion(const Rectangle &region) const
{
  const Int2 nsamples = count_samples_in_region(region);
  return nsamples[0] * nsamples[1];
}

} // namespace xxx
