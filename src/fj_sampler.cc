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
static int get_sample_count_for_region(int rate, int regionsize, int margin);

Sampler::Sampler() :
  xres_(1),
  yres_(1),
  xrate_(1),
  yrate_(1),
  xfwidth_(1),
  yfwidth_(1),
  jitter_(1),

  samples_(),

  xnsamples_(1),
  ynsamples_(1),
  xpixel_start_(0),
  ypixel_start_(0),
  xmargin_(0),
  ymargin_(0),
  xnpxlsmps_(1),
  ynpxlsmps_(1),

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

  xres_ = xres;
  yres_ = yres;
  xrate_ = xsamples;
  yrate_ = ysamples;
  xfwidth_ = xfwidth;
  yfwidth_ = yfwidth;
  SetJitter(1);
  SetSampleTimeRange(0, 1);
  samples_.clear();

  current_index_ = 0;

  count_samples_in_pixels();
}

void Sampler::SetJitter(float jitter)
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
  const int err = allocate_samples_for_region(pixel_bounds);
  if (err) {
    return -1;
  }

  XorShift xr; // random number generator
  XorShift rng_time; // for time sampling jitter
  XorInit(&rng_time);
  XorInit(&xr);

  // uv delta
  const Real udelta = 1./(xrate_ * xres_ + 2 * xmargin_);
  const Real vdelta = 1./(yrate_ * yres_ + 2 * ymargin_);

  // xy offset
  const int xoffset = xpixel_start_ * xrate_ - xmargin_;
  const int yoffset = ypixel_start_ * yrate_ - ymargin_;

  Sample *sample = &samples_[0];

  for (int y = 0; y < ynsamples_; y++) {
    for (int x = 0; x < xnsamples_; x++) {
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
  const int XPIXEL_OFFSET = pixel_x - xpixel_start_;
  const int YPIXEL_OFFSET = pixel_y - ypixel_start_;

  const int XNSAMPLES = xnsamples_;
  const int OFFSET =
    YPIXEL_OFFSET * yrate_ * XNSAMPLES +
    XPIXEL_OFFSET * xrate_;
  const Sample *src = &samples_[OFFSET];

  const std::size_t SAMPLE_COUNT = static_cast<std::size_t>(GetSampleCountForPixel());
  if (pixelsamples.size() < SAMPLE_COUNT) {
    pixelsamples.resize(SAMPLE_COUNT);
  }

  std::vector<Sample> &dst = pixelsamples;

  for (int y = 0; y < ynpxlsmps_; y++) {
    for (int x = 0; x < xnpxlsmps_; x++) {
      dst[y * xnpxlsmps_ + x] = src[y * XNSAMPLES + x];
    }
  }
}

int Sampler::GetSampleCountForPixel() const
{
  return xnpxlsmps_ * ynpxlsmps_;
}

// TODO REMOVE THIS OR MAKE FREE FUNCTION
int Sampler::GetSampleCountForRegion(const Rectangle &region,
    int xrate, int yrate, float xfwidth, float yfwidth)
{
  const int xmargin = get_pixel_margin(xrate, xfwidth);
  const int ymargin = get_pixel_margin(yrate, yfwidth);
  const int xnsamples = get_sample_count_for_region(xrate, SizeX(region), xmargin);
  const int ynsamples = get_sample_count_for_region(yrate, SizeY(region), ymargin);

  return xnsamples * ynsamples;
}

static int get_pixel_margin(int rate, float fwidth)
{
  return (int) ceil(((fwidth - 1) * rate) * .5);
}

void Sampler::count_samples_in_pixels()
{
  xmargin_ = get_pixel_margin(xrate_, xfwidth_);
  ymargin_ = get_pixel_margin(yrate_, yfwidth_);;
  xnpxlsmps_ = xrate_ + 2 * xmargin_;
  ynpxlsmps_ = yrate_ + 2 * ymargin_;
}

static int get_sample_count_for_region(int rate, int regionsize, int margin)
{
  return rate * regionsize + 2 * margin;
}

int Sampler::allocate_samples_for_region(const Rectangle &region)
{
  const int XNSAMPLES = get_sample_count_for_region(xrate_, SizeX(region), xmargin_);
  const int YNSAMPLES = get_sample_count_for_region(yrate_, SizeY(region), ymargin_);
  const int NEW_NSAMPLES = XNSAMPLES * YNSAMPLES;
  const int XPIXEL_START = region.xmin;
  const int YPIXEL_START = region.ymin;

  samples_.resize(NEW_NSAMPLES);

  xnsamples_ = XNSAMPLES;
  ynsamples_ = YNSAMPLES;
  xpixel_start_ = XPIXEL_START;
  ypixel_start_ = YPIXEL_START;

  current_index_ = 0;

  return 0;
}

} // namespace xxx
