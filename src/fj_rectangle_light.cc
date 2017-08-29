// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_rectangle_light.h"
#include "fj_multi_thread.h"

namespace fj {

RectangleLight::RectangleLight()
{
}

RectangleLight::~RectangleLight()
{
}

int RectangleLight::get_sample_count() const
{
  return sample_count_;
}

void RectangleLight::get_samples(LightSample *samples, int max_samples) const
{
  Transform transform_interp;
  // TODO time sampling
  XfmLerpTransformSample(&transform_samples_, 0, &transform_interp);

  Vector N_sample(0, 1, 0);
  XfmTransformVector(&transform_interp, &N_sample);
  N_sample = Normalize(N_sample);

  int nsamples = GetSampleCount();
  nsamples = Min(nsamples, max_samples);

  for (int i = 0; i < nsamples; i++) {
    XorShift &mutable_rng = const_cast<XorShift &>(rng_);
    const Real x = mutable_rng.NextFloat01() - .5;
    const Real z = mutable_rng.NextFloat01() - .5;
    Vector P_sample;
    P_sample.x = x;
    P_sample.z = z;

    XfmTransformPoint(&transform_interp, &P_sample);

    samples[i].P = P_sample;
    samples[i].N = N_sample;
    samples[i].light = this;
  }
}

Color RectangleLight::illuminate(const LightSample &sample, const Vector &Ps) const
{
  Vector Ln = Ps - sample.P;
  Ln = Normalize(Ln);

  Color Cl;

  Real dot = Dot(Ln, sample.N);
  if (double_sided_) {
    dot = Abs(dot);
  } else {
    dot = Max(dot, 0.);
  }

  Cl = sample_intensity_ * color_;
  Cl *= dot;

  return Cl;
}

int RectangleLight::preprocess()
{
  // does nothing
  return 0;
}

} // namespace xxx
