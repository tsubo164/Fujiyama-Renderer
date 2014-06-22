// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_light.h"
#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_numeric.h"
#include "fj_texture.h"
#include <cfloat>

namespace fj {

static int point_light_get_sample_count(const Light *light);
static void point_light_get_samples(const Light *light,
    LightSample *samples, int max_samples);
static void point_light_illuminate(const Light *light,
    const LightSample *sample,
    const Vector *Ps, Color *Cl);

static int grid_light_get_sample_count(const Light *light);
static void grid_light_get_samples(const Light *light,
    LightSample *samples, int max_samples);
static void grid_light_illuminate(const Light *light,
    const LightSample *sample,
    const Vector *Ps, Color *Cl);

static int sphere_light_get_sample_count(const Light *light);
static void sphere_light_get_samples(const Light *light,
    LightSample *samples, int max_samples);
static void sphere_light_illuminate(const Light *light,
    const LightSample *sample,
    const Vector *Ps, Color *Cl);

static int dome_light_get_sample_count(const Light *light);
static void dome_light_get_samples(const Light *light,
    LightSample *samples, int max_samples);
static void dome_light_illuminate(const Light *light,
    const LightSample *sample,
    const Vector *Ps, Color *Cl);
static int dome_light_preprocess(Light *light);

static int no_preprocess(Light *light);

Light::Light() :
  color_(1, 1, 1),
  intensity_(1),
  transform_samples_(),
  xr_(),

  type_(LGT_POINT),
  double_sided_(false),
  sample_count_(16),
  sample_intensity_(intensity_ / sample_count_),

  environment_map_(NULL),
  dome_samples_(),

  GetSampleCount_(NULL),
  GetSamples_(NULL),
  Illuminate_(NULL),
  Preprocess_(NULL)
{
  SetLightType(LGT_POINT);
}

Light::~Light()
{
}

void Light::SetLightType(int light_type)
{
  type_ = light_type;

  XfmInitTransformSampleList(&transform_samples_);
  XorInit(&xr_);

  switch (type_) {
  case LGT_POINT:
    GetSampleCount_ = point_light_get_sample_count;
    GetSamples_     = point_light_get_samples;
    Illuminate_     = point_light_illuminate;
    Preprocess_     = no_preprocess;
    break;
  case LGT_GRID:
    GetSampleCount_ = grid_light_get_sample_count;
    GetSamples_     = grid_light_get_samples;
    Illuminate_     = grid_light_illuminate;
    Preprocess_     = no_preprocess;
    break;
  case LGT_SPHERE:
    GetSampleCount_ = sphere_light_get_sample_count;
    GetSamples_     = sphere_light_get_samples;
    Illuminate_     = sphere_light_illuminate;
    Preprocess_     = no_preprocess;
    break;
  case LGT_DOME:
    GetSampleCount_ = dome_light_get_sample_count;
    GetSamples_     = dome_light_get_samples;
    Illuminate_     = dome_light_illuminate;
    Preprocess_     = dome_light_preprocess;
    break;
  default:
    // TODO should abort()?
    type_ = LGT_POINT;
    GetSampleCount_ = point_light_get_sample_count;
    GetSamples_     = point_light_get_samples;
    Illuminate_     = point_light_illuminate;
    Preprocess_     = no_preprocess;
    break;
  }
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
  GetSamples_(this, samples, max_samples);
}

int Light::GetSampleCount() const
{
  return GetSampleCount_(this);
}

Color Light::Illuminate(const LightSample &sample, const Vector &Ps) const
{
  Color Cl;
  Illuminate_(this, &sample, &Ps, &Cl);
  return Cl;
}

int Light::Preprocess()
{
  return Preprocess_(this);
}

Light *LgtNew(int light_type)
{
  Light *light = new Light();
  light->SetLightType(light_type);
  return light;
}

void LgtFree(Light *light)
{
  delete light;
}

// point light
static int point_light_get_sample_count(const Light *light)
{
  return 1;
}

static void point_light_get_samples(const Light *light,
    LightSample *samples, int max_samples)
{
  if (max_samples == 0)
    return;

  Transform transform_interp;
  // TODO time sampling
  XfmLerpTransformSample(&light->transform_samples_, 0, &transform_interp);

  samples[0].P = transform_interp.translate;
  samples[0].N = Vector(0, 0, 0);
  samples[0].light = light;
}

static void point_light_illuminate(const Light *light,
    const LightSample *sample,
    const Vector *Ps, Color *Cl)
{
  *Cl = light->intensity_ * light->color_;
}

// grid light
static int grid_light_get_sample_count(const Light *light)
{
  return light->sample_count_;
}

static void grid_light_get_samples(const Light *light,
    LightSample *samples, int max_samples)
{
  Transform transform_interp;
  // TODO time sampling
  XfmLerpTransformSample(&light->transform_samples_, 0, &transform_interp);

  Vector N_sample(0, 1, 0);
  XfmTransformVector(&transform_interp, &N_sample);
  Normalize(&N_sample);

  int nsamples = light->GetSampleCount();
  nsamples = Min(nsamples, max_samples);

  for (int i = 0; i < nsamples; i++) {
    XorShift *mutable_xr = (XorShift *) &light->xr_;
    const Real x = (XorNextFloat01(mutable_xr) - .5);
    const Real z = (XorNextFloat01(mutable_xr) - .5);
    Vector P_sample;
    P_sample.x = x;
    P_sample.z = z;

    XfmTransformPoint(&transform_interp, &P_sample);

    samples[i].P = P_sample;
    samples[i].N = N_sample;
    samples[i].light = light;
  }
}

static void grid_light_illuminate(const Light *light,
    const LightSample *sample,
    const Vector *Ps, Color *Cl)
{
  Vector Ln = *Ps - sample->P;
  Normalize(&Ln);

  Real dot = Dot(Ln, sample->N);
  if (light->double_sided_) {
    dot = Abs(dot);
  } else {
    dot = Max(dot, 0.);
  }

  *Cl = light->sample_intensity_ * light->color_;
  *Cl *= dot;
}

// sphere light
static int sphere_light_get_sample_count(const Light *light)
{
  return light->sample_count_;
}

static void sphere_light_get_samples(const Light *light,
    LightSample *samples, int max_samples)
{
  Transform transform_interp;
  // TODO time sampling
  XfmLerpTransformSample(&light->transform_samples_, 0, &transform_interp);

  int nsamples = light->GetSampleCount();
  nsamples = Min(nsamples, max_samples);

  for (int i = 0; i < nsamples; i++) {
    XorShift *mutable_xr = (XorShift *) &light->xr_;
    Vector P_sample;
    Vector N_sample;

    XorHollowSphereRand(mutable_xr, &P_sample);
    N_sample = P_sample;

    XfmTransformPoint(&transform_interp, &P_sample);
    XfmTransformVector(&transform_interp, &N_sample);
    Normalize(&N_sample);

    samples[i].P = P_sample;
    samples[i].N = N_sample;
    samples[i].light = light;
  }
}

static void sphere_light_illuminate(const Light *light,
    const LightSample *sample,
    const Vector *Ps, Color *Cl)
{
  Vector Ln = *Ps - sample->P;
  Normalize(&Ln);

  const Real dot = Dot(Ln, sample->N);
  if (dot > 0) {
    *Cl = light->sample_intensity_ * light->color_;
  } else {
    *Cl = Color();
  }
}

// dome light
static int dome_light_get_sample_count(const Light *light)
{
  return light->sample_count_;
}

static void dome_light_get_samples(const Light *light,
    LightSample *samples, int max_samples)
{
  Transform transform_interp;
  // TODO time sampling
  XfmLerpTransformSample(&light->transform_samples_, 0, &transform_interp);

  int nsamples = light->GetSampleCount();
  nsamples = Min(nsamples, max_samples);

  for (int i = 0; i < nsamples; i++) {
    const DomeSample *dome_sample = &light->dome_samples_[i];

    // TODO CHANGE IT TO REAL_MAX WHEN FINISHING IT TO OTHERS
    Vector P_sample = dome_sample->dir * FLT_MAX;
    Vector N_sample = -1 * dome_sample->dir;

    // TODO cancel translate and scale
    XfmTransformPoint(&transform_interp, &P_sample);
    XfmTransformVector(&transform_interp, &N_sample);

    samples[i].P = P_sample;
    samples[i].N = N_sample;
    samples[i].color = dome_sample->color;
    samples[i].light = light;
  }
}

static void dome_light_illuminate(const Light *light,
    const LightSample *sample,
    const Vector *Ps, Color *Cl)
{
  *Cl = light->sample_intensity_ * sample->color;
}

static int dome_light_preprocess(Light *light)
{
  const int NSAMPLES = light->GetSampleCount();
  DomeSample init_sample;
  init_sample.uv = TexCoord(1./NSAMPLES, 1./NSAMPLES);
  init_sample.color = Color(1, .63, .63);
  init_sample.dir = Vector(1./NSAMPLES, 1, 1./NSAMPLES);
  Normalize(&init_sample.dir);

  light->dome_samples_.resize(NSAMPLES, init_sample);
  std::vector<DomeSample>(light->dome_samples_).swap(light->dome_samples_);

  if (light->environment_map_ == NULL) {
    // TODO should be an error?
    return 0;
  }

  int XRES = light->environment_map_->GetWidth();
  int YRES = light->environment_map_->GetHeight();
  // TODO parameteraize
  XRES /= 8;
  YRES /= 8;

  if (0) {
    ImportanceSampling(light->environment_map_, 0,
        XRES, YRES,
        &light->dome_samples_[0], NSAMPLES);
  } else if (1) {
    StratifiedImportanceSampling(light->environment_map_, 0,
        XRES, YRES,
        &light->dome_samples_[0], NSAMPLES);
  } else {
    StructuredImportanceSampling(light->environment_map_, 0,
        XRES, YRES,
        &light->dome_samples_[0], NSAMPLES);
  }

  return 0;
}

static int no_preprocess(Light *light)
{
  // does nothing
  return 0;
}

} // namespace xxx
