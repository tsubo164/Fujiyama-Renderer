// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_light.h"
#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_numeric.h"
#include "fj_texture.h"

namespace fj {

Light::Light() :
  color_(1, 1, 1),
  intensity_(1),
  transform_samples_(),

  double_sided_(false),
  sample_count_(16),
  sample_intensity_(intensity_ / sample_count_),

  environment_map_(NULL)
{
  XfmInitTransformSampleList(&transform_samples_);
}

Light::~Light()
{
}

void Light::SetColor(float r, float g, float b)
{
  color_ = Color(r, g, b);
}

void Light::SetIntensity(float intensity)
{
  intensity_ = intensity;
  // TODO temp
  sample_intensity_ = intensity_ / sample_count_;
}

void Light::SetSampleCount(int sample_count)
{
  sample_count_ = Max(sample_count, 1);
  // TODO temp
  sample_intensity_ = intensity_ / sample_count_;
}

void Light::SetDoubleSided(bool on_or_off)
{
  double_sided_ = on_or_off;
}

void Light::SetEnvironmentMap(Texture *texture)
{
  environment_map_ = texture;
}

Color Light::GetColor() const
{
  return color_;
}

float Light::GetIntensity() const
{
  return intensity_;
}

int Light::GetSampleDensity() const
{
  return sample_count_;
}

bool Light::IsDoulbeSided() const
{
  return double_sided_;
}

Texture *Light::GetEnvironmentMap() const
{
  return environment_map_;
}

void Light::SetTranslate(Real tx, Real ty, Real tz, Real time)
{
  XfmPushTranslateSample(&transform_samples_, tx, ty, tz, time);
}

void Light::SetRotate(Real rx, Real ry, Real rz, Real time)
{
  XfmPushRotateSample(&transform_samples_, rx, ry, rz, time);
}

void Light::SetScale(Real sx, Real sy, Real sz, Real time)
{
  XfmPushScaleSample(&transform_samples_, sx, sy, sz, time);
}

void Light::SetTransformOrder(int order)
{
  XfmSetSampleTransformOrder(&transform_samples_, order);
}

void Light::SetRotateOrder(int order)
{
  XfmSetSampleRotateOrder(&transform_samples_, order);
}

void Light::GetSamples(LightSample *samples, int max_samples) const
{
  get_samples(samples, max_samples);
}

int Light::GetSampleCount() const
{
  return get_sample_count();
}

Color Light::Illuminate(const LightSample &sample, const Vector &Ps) const
{
  return illuminate(sample, Ps);
}

int Light::Preprocess()
{
  return preprocess();
}

void Light::get_transform_sample(Transform &sample, Real time) const
{
  // TODO time sampling
  XfmLerpTransformSample(&transform_samples_, time, &sample);
}

float Light::get_sample_intensity() const
{
  return sample_intensity_;
}

} // namespace xxx
