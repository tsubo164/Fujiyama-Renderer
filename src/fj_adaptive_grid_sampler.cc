// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_adaptive_grid_sampler.h"
#include "fj_rectangle.h"
#include "fj_numeric.h"
#include "fj_random.h"

namespace fj {

AdaptiveGridSampler::AdaptiveGridSampler() :
  samples_(),

  nsamples_(1, 1),
  pixel_start_(0, 0),
  margin_(0, 0),
  npxlsmps_(1, 1),
  ndivision_(2, 2),

  current_index_(0)
{
}

AdaptiveGridSampler::~AdaptiveGridSampler()
{
}

void AdaptiveGridSampler::update_sample_counts()
{
  margin_ = count_samples_in_margin();
  npxlsmps_ = count_samples_in_pixel();
  // TODO ADAPTIVE_TEST
  ndivision_ = compute_num_pixel_division();
}

int AdaptiveGridSampler::generate_samples(const Rectangle &region)
{
  // TODO ADAPTIVE_TEST
  curr_pixel_ = region.min;

  // allocate samples in region
  nsamples_ = count_samples_in_region(region);
  samples_.resize(nsamples_[0] * nsamples_[1]);
  pixel_start_ = region.min;
  current_index_ = 0;

  XorShift rng; // random number generator
  XorShift rng_time; // for time sampling jitter

  // TODO ADAPTIVE_TEST
  const Int2 rate = ndivision_; //Int2(2, 2); //GetPixelSamples();
  const Int2 res  = GetResolution();
  const Real jitter = GetJitter();
  const Vector2 sample_time_range = GetSampleTimeRange();

  // uv delta (screen space uv. excludes margins)
  const Real udelta = 1./(rate[0] * res[0]);
  const Real vdelta = 1./(rate[1] * res[1]);

  // xy offset
  const int xoffset = pixel_start_[0] * rate[0] - 0 * margin_[0];
  const int yoffset = pixel_start_[1] * rate[1] - 0 * margin_[1];

  Sample *sample = &samples_[0];
  //std::cout << "sample count: " << samples_.size() << "\n";
  //std::cout << "current pixel: " << curr_pixel_ << "\n";
  // TODO ADAPTIVE_TEST
  subd_flag_.resize(samples_.size(), 0);

  for (int y = 0; y < nsamples_[1]; y++) {
    for (int x = 0; x < nsamples_[0]; x++) {
      // TODO ADAPTIVE_TEST
      sample->uv.x =     (0*.5 + x + xoffset) * udelta;
      sample->uv.y = 1 - (0*.5 + y + yoffset) * vdelta;

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
      sample->data = Vector4(0, 1, 0, -1);
      sample++;
    }
  }
  // TODO ADAPTIVE_TEST
  rect_stack_ = Stack();
  Int2 tile_size = region.Size();
        //std::cout << "tile_size: " << tile_size << "\n";
  for (int y = 0; y < tile_size[1]; y++) {
    for (int x = 0; x < tile_size[0]; x++) {
      Rectangle r;
      r.min = Int2(x, y) * ndivision_;
      r.max = r.min + ndivision_;
      rect_stack_.push(r);
      if (x == 31) {
        //std::cout << "rect: " << r << "\n";
      }
    }
  }
  return 0;
}
#if 0
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
#endif

Sample *AdaptiveGridSampler::get_next_sample()
{
  // TODO ADAPTIVE_TEST
  while (!rect_stack_.empty()) {
    const Rectangle rect = rect_stack_.top();
    Int2 s;
    switch (curr_corner_) {
      case 0: s = Int2(rect.min[0], rect.min[1]); break;
      case 1: s = Int2(rect.max[0], rect.min[1]); break;
      case 2: s = Int2(rect.min[0], rect.max[1]); break;
      case 3: s = Int2(rect.max[0], rect.max[1]); break;
      default: break;
    }

    curr_corner_++;
    if (curr_corner_ == 4) {
      curr_corner_ = 0;
      rect_stack_.pop();

      if (need_subd_rect(rect)) {
        //std::cout << "SUBD!!\n";
      } else {
        //std::cout << "NO SUBD!!\n";
      }
    }

    //const Int2 rate = Int2(2, 2); //GetPixelSamples();
    //const Int2 tile_size = Int2(32, 32);
    const int OFFSET = s[1] * nsamples_[0] + s[0];

    Sample *src = &samples_[OFFSET];
    if (src->data[3] < 0) {
      return src;
    }
  }

  int j = 0;
  for (std::size_t i = 0; i < samples_.size(); i++) {
    if (samples_[i].data[3] == -1) {
      j++;
    }
    samples_[i].data[3] = subd_flag_[i];
  }

  return NULL;
#if 0

  if (current_index_ >= get_sample_count())
    return NULL;

  Sample *sample = &samples_[current_index_];
  current_index_++;

  return sample;
#endif
}

void AdaptiveGridSampler::get_sampleset_in_pixel(std::vector<Sample> &pixelsamples,
    const Int2 &pixel_pos) const
{
  // TODO ADAPTIVE_TEST
  const Int2 rate = ndivision_; //Int2(2, 2); //GetPixelSamples();

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
#if 0
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
#endif
}

int AdaptiveGridSampler::get_sample_count() const
{
  return samples_.size();
}

Int2 AdaptiveGridSampler::count_samples_in_margin() const
{
  return Int2(
      static_cast<int>(Ceil(((GetFilterWidth()[0] - 1) * GetPixelSamples()[0]) * .5)),
      static_cast<int>(Ceil(((GetFilterWidth()[1] - 1) * GetPixelSamples()[1]) * .5)));
}

Int2 AdaptiveGridSampler::count_samples_in_pixel() const
{
  //return  GetPixelSamples() + 2 * margin_;
  // TODO ADAPTIVE_TEST
  return ndivision_  + 0 * 2 * margin_ + Int2(1, 1);
  //return Int2(2, 2)  + 0 * 2 * margin_ + Int2(1, 1);
}

Int2 AdaptiveGridSampler::count_samples_in_region(const Rectangle &region) const
{
  //return GetPixelSamples() * region.Size() + 2 * margin_;
  // TODO ADAPTIVE_TEST
  return ndivision_ * region.Size() + 0 * 2 * margin_ + Int2(1, 1);
  //return Int2(2, 2) * region.Size() + 0 * 2 * margin_ + Int2(1, 1);
}

Int2 AdaptiveGridSampler::compute_num_pixel_division() const
{
  const int nsubd = std::pow(GetMaxSubdivision(), 2) + 1;
  return Int2(nsubd, nsubd);
}

bool AdaptiveGridSampler::need_subd_rect(const Rectangle rect)
{
  Int2 corner[4];
  corner[0] = Int2(rect.min[0], rect.min[1]);
  corner[1] = Int2(rect.max[0], rect.min[1]);
  corner[2] = Int2(rect.min[0], rect.max[1]);
  corner[3] = Int2(rect.max[0], rect.max[1]);

  Vector4 min(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
  Vector4 max(FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN);

  for (int i = 1; i < 4; i++) {
    const int OFFSET = corner[i][1] * nsamples_[0] + corner[i][0];
    const Sample &smp = samples_[OFFSET];

    min[0] = Min(min[0], smp.data[0]);
    min[1] = Min(min[1], smp.data[1]);
    min[2] = Min(min[2], smp.data[2]);
    min[3] = Min(min[3], smp.data[3]);

    max[0] = Max(max[0], smp.data[0]);
    max[1] = Max(max[1], smp.data[1]);
    max[2] = Max(max[2], smp.data[2]);
    max[3] = Max(max[3], smp.data[3]);
  }

  const Int2 size = rect.Size();
  if (size[0] > 1 || size[1] < 1) {

    for (int i = 0; i < 4; i++) {
      if (max[i] - min[i] > .05) {
        //----
        for (int i = 0; i < 4; i++) {
          const int OFFSET = corner[i][1] * nsamples_[0] + corner[i][0];
          subd_flag_[OFFSET] = 1;
        }
        //----

        /*
        corner[0] - new_pt[0] - corner[1]
           |           |           |
        new_pt[1] - new_pt[4] - new_pt[3]
           |           |           |
        corner[2] - new_pt[2] - corner[3]
        */
#if 0
#endif
        Int2 new_pt[5];
        new_pt[0] = (corner[0] + corner[1]) / 2;
        new_pt[1] = (corner[0] + corner[2]) / 2;
        new_pt[2] = (corner[2] + corner[3]) / 2;
        new_pt[3] = (corner[1] + corner[3]) / 2;
        new_pt[4] = (new_pt[0] + new_pt[2]) / 2;

        Rectangle r;
        r.min = corner[0];
        r.max = new_pt[4];
        rect_stack_.push(r);

        r.min = new_pt[0];
        r.max = new_pt[3];
        rect_stack_.push(r);

        r.min = new_pt[1];
        r.max = new_pt[2];
        rect_stack_.push(r);

        r.min = new_pt[4];
        r.max = corner[3];
        rect_stack_.push(r);

        return true;
      }
    }

  }
      //----
      for (int i = 0; i < 4; i++) {
        const int OFFSET = corner[i][1] * nsamples_[0] + corner[i][0];
        subd_flag_[OFFSET] = 0;
      }
      //----

      const int XMIN = rect.min[0], XMAX = rect.max[0];
      const int YMIN = rect.min[1], YMAX = rect.max[1];

      const Sample &smp0 = samples_[YMIN * nsamples_[0] + XMIN];
      const Sample &smp1 = samples_[YMIN * nsamples_[0] + XMAX];
      const Sample &smp2 = samples_[YMAX * nsamples_[0] + XMIN];
      const Sample &smp3 = samples_[YMAX * nsamples_[0] + XMAX];

      // using less than equal
      for (int y = YMIN; y <= YMAX; y++) {
        const Vector4 data02 = Lerp(smp0.data, smp2.data, 1.*(y - YMIN) / (YMAX-YMIN));
        const Vector4 data13 = Lerp(smp1.data, smp3.data, 1.*(y - YMIN) / (YMAX-YMIN));

        // using less than equal
        for (int x = XMIN; x <= XMAX; x++) {
          const int OFFSET = y * nsamples_[0] + x;
          Sample &smp = samples_[OFFSET];

          smp.data = Lerp(data02, data13, 1.*(x - XMIN) / (XMAX-XMIN));
        }
      }
  return false;
}

} // namespace xxx
