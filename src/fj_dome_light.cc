// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_dome_light.h"
#include "fj_multi_thread.h"
#include "fj_texture.h"

namespace fj {

DomeLight::DomeLight() : dome_samples_()
{
}

DomeLight::~DomeLight()
{
}

int DomeLight::get_sample_count() const
{
  return GetSampleDensity();
}

void DomeLight::get_samples(LightSample *samples, int max_samples) const
{
  Transform transform_interp;
  // TODO time sampling
  const float time = 0;
  get_transform_sample(transform_interp, time);

  int nsamples = GetSampleCount();
  nsamples = Min(nsamples, max_samples);

  for (int i = 0; i < nsamples; i++) {
    const DomeSample *dome_sample = &dome_samples_[i];

    // TODO CHANGE IT TO REAL_MAX WHEN FINISHING IT TO OTHERS
    Vector P_sample = dome_sample->dir * FLT_MAX;
    Vector N_sample = -1 * dome_sample->dir;

    // TODO cancel translate and scale
    XfmTransformPoint(&transform_interp, &P_sample);
    XfmTransformVector(&transform_interp, &N_sample);

    samples[i].P = P_sample;
    samples[i].N = N_sample;
    samples[i].color = dome_sample->color;
    samples[i].light = this;
  }
}

Color DomeLight::illuminate(const LightSample &sample, const Vector &Ps) const
{
  return get_sample_intensity() * sample.color;
}

int DomeLight::preprocess()
{
  const int NSAMPLES = GetSampleCount();
  DomeSample init_sample;
  init_sample.uv = TexCoord(1./NSAMPLES, 1./NSAMPLES);
  init_sample.color = Color(1, .63, .63);
  init_sample.dir = Vector(1./NSAMPLES, 1, 1./NSAMPLES);
  init_sample.dir = Normalize(init_sample.dir);

  dome_samples_.resize(NSAMPLES, init_sample);
  std::vector<DomeSample>(dome_samples_).swap(dome_samples_);

  Texture *envmap = GetEnvironmentMap();
  if (envmap == NULL) {
    // TODO should be an error?
    return 0;
  }

  int XRES = envmap->GetWidth();
  int YRES = envmap->GetHeight();
  // TODO parameteraize
  XRES /= 8;
  YRES /= 8;

  if (0) {
    ImportanceSampling(envmap, 0,
        XRES, YRES,
        &dome_samples_[0], NSAMPLES);
  } else if (1) {
    StratifiedImportanceSampling(envmap, 0,
        XRES, YRES,
        &dome_samples_[0], NSAMPLES);
  } else {
    StructuredImportanceSampling(envmap, 0,
        XRES, YRES,
        &dome_samples_[0], NSAMPLES);
  }

  return 0;
}

} // namespace xxx
