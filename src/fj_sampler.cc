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

static int get_pixel_margin(int rate, float fwidth);
static Int2 get_sample_count_for_region(const Rectangle &region,
    const Int2 &rate, const Int2 &margin);

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

void Sampler::Initialize(int xres, int yres,
    int xsamples, int ysamples, float xfwidth, float yfwidth)
{
  assert(xres > 0);
  assert(yres > 0);
  assert(xsamples > 0);
  assert(ysamples > 0);
  assert(xfwidth > 0);
  assert(yfwidth > 0);

  res_  = Int2(xres, yres);
  rate_ = Int2(xsamples, ysamples);
  fwidth_ = Vector2(xfwidth, yfwidth);
  SetJitter(1.);
  SetSampleTimeRange(0, 1);
  samples_.clear();

  current_index_ = 0;

  count_samples_in_pixels();
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

int Sampler::GenerateSamples(const Rectangle &pixel_bounds)
{
  allocate_samples_for_region(pixel_bounds);

  XorShift xr; // random number generator
  XorShift rng_time; // for time sampling jitter
  XorInit(&rng_time);
  XorInit(&xr);

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
        const Real u_jitter = XorNextFloat01(&xr) * jitter_;
        const Real v_jitter = XorNextFloat01(&xr) * jitter_;

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

int Sampler::GetSampleCount() const
{
  return samples_.size();
}

Sample *Sampler::GetNextSample()
{
  if (current_index_ >= GetSampleCount())
    return NULL;

  Sample *sample = &samples_[current_index_];
  current_index_++;

  return sample;
}

void Sampler::GetSampleSetForPixel(std::vector<Sample> &pixelsamples,
    int pixel_x, int pixel_y) const
{
  const int XPIXEL_OFFSET = pixel_x - pixel_start_[0];
  const int YPIXEL_OFFSET = pixel_y - pixel_start_[1];

  const int XNSAMPLES = nsamples_[0];
  const int OFFSET =
    YPIXEL_OFFSET * rate_[1] * XNSAMPLES +
    XPIXEL_OFFSET * rate_[0];
  const Sample *src = &samples_[OFFSET];

  const std::size_t SAMPLE_COUNT = static_cast<std::size_t>(GetSampleCountForPixel());
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

int Sampler::GetSampleCountForPixel() const
{
  return npxlsmps_[0] * npxlsmps_[1];
}

int Sampler::ComputeSampleCountForRegion(const Rectangle &region) const
{
  const Int2 nsamples = get_sample_count_for_region(region, rate_, margin_);
  return nsamples[0] * nsamples[1];
}

static int get_pixel_margin(int rate, float fwidth)
{
  return static_cast<int>(Ceil(((fwidth - 1) * rate) * .5));
}

void Sampler::count_samples_in_pixels()
{
  margin_[0] = get_pixel_margin(rate_[0], fwidth_[0]);
  margin_[1] = get_pixel_margin(rate_[1], fwidth_[1]);;
  npxlsmps_[0] = rate_[0] + 2 * margin_[0];
  npxlsmps_[1] = rate_[1] + 2 * margin_[1];
}

int Sampler::allocate_samples_for_region(const Rectangle &region)
{
  nsamples_ = get_sample_count_for_region(region, rate_, margin_);
  samples_.resize(nsamples_[0] * nsamples_[1]);

  pixel_start_ = Int2(region.xmin, region.ymin);

  current_index_ = 0;

  return 0;
}

static Int2 get_sample_count_for_region(const Rectangle &region,
    const Int2 &rate, const Int2 &margin)
{
  return Int2(
      rate[0] * SizeX(region) + 2 * margin[0],
      rate[1] * SizeY(region) + 2 * margin[1]);
}

} // namespace xxx
