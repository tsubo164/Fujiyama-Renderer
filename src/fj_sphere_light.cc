// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_sphere_light.h"
#include "fj_multi_thread.h"

namespace fj {

SphereLight::SphereLight()
{
}

SphereLight::~SphereLight()
{
}

int SphereLight::get_sample_count() const
{
  return sample_count_;
}

void SphereLight::get_samples(LightSample *samples, int max_samples) const
{
  Transform transform_interp;
  // TODO time sampling
  XfmLerpTransformSample(&transform_samples_, 0, &transform_interp);

  int nsamples = GetSampleCount();
  nsamples = Min(nsamples, max_samples);

  for (int i = 0; i < nsamples; i++) {
    XorShift &mutable_rng = const_cast<XorShift &>(rng_);
    Vector P_sample;
    Vector N_sample;

    P_sample = mutable_rng.HollowSphereRand();
    N_sample = P_sample;

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
  Vector Ln = Ps - sample.P;
  Ln = Normalize(Ln);

  Color Cl;

  const Real dot = Dot(Ln, sample.N);
  if (dot > 0) {
    Cl = sample_intensity_ * color_;
  } else {
    Cl = Color();
  }

  return Cl;
}

#if 0
void SphereLight::get_light_samples(std::vector<LightSample> &samples) const
{
  static XorShift rng_bank_[64];
  XorShift &my_rng_ = rng_bank_[MtGetThreadID()];
  int max_samples = 16;

  Transform transform_interp;
  // TODO time sampling
  XfmLerpTransformSample(&transform_samples_, 0, &transform_interp);

  int nsamples = GetSampleCount();
  nsamples = Min(nsamples, max_samples);

  for (int i = 0; i < nsamples; i++) {
    XorShift &mutable_rng = const_cast<XorShift &>(my_rng_);
    Vector P_sample;
    Vector N_sample;

    P_sample = mutable_rng.HollowSphereRand();
    N_sample = P_sample;

    XfmTransformPoint(&transform_interp, &P_sample);
    XfmTransformVector(&transform_interp, &N_sample);
    N_sample = Normalize(N_sample);

    samples[i].P = P_sample;
    samples[i].N = N_sample;
    samples[i].light = this;
  }
}
#endif

} // namespace xxx
