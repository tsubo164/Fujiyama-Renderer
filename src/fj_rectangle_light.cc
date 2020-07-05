// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_rectangle_light.h"

namespace fj {

RectangleLight::RectangleLight() : rng_()
{
}

RectangleLight::~RectangleLight()
{
}

int RectangleLight::get_sample_count() const
{
  return GetSampleDensity();
}

void RectangleLight::get_samples(LightSample *samples, int max_samples) const
{
  Transform transform_interp;
  // TODO time sampling
  const float time = 0;
  get_transform_sample(transform_interp, time);

  Vector N_sample(0, 1, 0);
  XfmTransformVector(&transform_interp, &N_sample);
  N_sample = Normalize(N_sample);

  int nsamples = GetSampleCount();
  nsamples = Min(nsamples, max_samples);

  for (int i = 0; i < nsamples; i++) {
    XorShift &mutable_rng = const_cast<XorShift &>(rng_);
    const Real x = mutable_rng.NextFloat01() - .5;
    const Real z = mutable_rng.NextFloat01() - .5;
    Vector P_sample(x, 0, z);

    XfmTransformPoint(&transform_interp, &P_sample);

    samples[i].P = P_sample;
    samples[i].N = N_sample;
    samples[i].light = this;
  }
}

Color RectangleLight::illuminate(const LightSample &sample, const Vector &Ps) const
{
  const Vector Ln = Normalize(Ps - sample.P);

  Real dot = Dot(Ln, sample.N);
  if (IsDoulbeSided()) {
    dot = Abs(dot);
  } else {
    dot = Max(dot, 0.);
  }

  return dot * get_sample_intensity() * GetColor();
}

int RectangleLight::preprocess()
{
  // does nothing
  return 0;
}

} // namespace xxx
