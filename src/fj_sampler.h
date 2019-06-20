// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SAMPLER_H
#define FJ_SAMPLER_H

#include "fj_pixel_sample.h"
#include "fj_vector.h"
#include "fj_types.h"
#include <vector>

namespace fj {

class Rectangle;

class Sampler {
public:
  Sampler();
  virtual ~Sampler();

  void SetResolution(const Int2 &resolution);
  void SetPixelSamples(const Int2 &pixel_samples);
  void SetFilterWidth(const Vector2 &filter_width);
  // TODO ADAPTIVE_TEST
  void SetMaxSubdivision(int max_subd);
  void SetSubdivisionThreshold(Real subd_threshold);

  void SetJitter(Real jitter);
  void SetSampleTimeRange(Real start_time, Real end_time);

  const Int2    &GetResolution() const;
  const Int2    &GetPixelSamples() const;
  const Vector2 &GetFilterWidth() const;
  // TODO ADAPTIVE_TEST
  int            GetMaxSubdivision() const;
  Real           GetSubdivisionThreshold() const;

  Vector2 GetSampleTimeRange() const;
  bool IsSamplingTime() const;
  bool IsJittered() const;
  Real GetJitter() const;

  int GenerateSamples(const Rectangle &region);
  Sample *GetNextSample();
  void GetSampleSetInPixel(std::vector<Sample> &pixelsamples,
      int pixel_x, int pixel_y) const;

private:
  virtual void update_sample_counts() = 0;
  virtual int generate_samples(const Rectangle &region) = 0;
  virtual Sample *get_next_sample() = 0;
  virtual void get_sampleset_in_pixel(std::vector<Sample> &pixelsamples,
      const Int2 &pixel_pos) const = 0;

  Int2 res_;
  Int2 rate_;
  Vector2 fwidth_;
  Real jitter_;
  int  max_subd_;
  Real subd_threshold_;

  bool need_jitter_;
  bool need_time_sampling_;

  Real sample_time_start_;
  Real sample_time_end_;
};

} // namespace xxx

#endif // FJ_XXX_H
