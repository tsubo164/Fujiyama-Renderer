// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_point_light.h"

namespace fj {

PointLight::PointLight()
{
}

PointLight::~PointLight()
{
}

int PointLight::get_sample_count() const
{
  return 1;
}

void PointLight::get_samples(LightSample *samples, int max_samples) const
{
  if (max_samples == 0)
    return;

  Transform transform_interp;
  // TODO time sampling
  XfmLerpTransformSample(&transform_samples_, 0, &transform_interp);

  samples[0].P = transform_interp.translate;
  samples[0].N = Vector(0, 0, 0);
  samples[0].light = this;
}

Color PointLight::illuminate(const LightSample &sample, const Vector &Ps) const
{
  return intensity_ * color_;
}

int PointLight::preprocess()
{
  // does nothing
  return 0;
}

} // namespace xxx
