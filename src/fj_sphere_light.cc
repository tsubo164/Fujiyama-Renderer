// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_sphere_light.h"
#include "fj_multi_thread.h"

namespace fj {

SphereLight::SphereLight() : rng_()
{
}

SphereLight::~SphereLight()
{
}

int SphereLight::get_sample_count() const
{
  return GetSampleDensity();
}

void SphereLight::get_samples(LightSample *samples, int max_samples) const
{
  Transform transform_interp;
  // TODO time sampling
  const float time = 0;
  get_transform_sample(transform_interp, time);

  int nsamples = GetSampleCount();
  nsamples = Min(nsamples, max_samples);

  for (int i = 0; i < nsamples; i++) {
    XorShift &mutable_rng = const_cast<XorShift &>(rng_);
    Vector P_sample = mutable_rng.HollowSphereRand();
    Vector N_sample = P_sample;

    XfmTransformPoint(&transform_interp, &P_sample);
    XfmTransformVector(&transform_interp, &N_sample);
    N_sample = Normalize(N_sample);

    samples[i].P = P_sample;
    samples[i].N = N_sample;
    samples[i].light = this;
  }
}

Color SphereLight::illuminate(const LightSample &sample, const Vector &Ps) const
{
  const Vector Ln = Normalize(Ps - sample.P);
  Color Cl;

  const Real dot = Dot(Ln, sample.N);
  if (dot > 0) {
    Cl = get_sample_intensity() * GetColor();
    //Cl *= dot;
  }

  return Cl;
}

int SphereLight::preprocess()
{
  // does nothing
  return 0;
}

} // namespace xxx
