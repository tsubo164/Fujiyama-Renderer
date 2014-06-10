/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_light.h"
#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_numeric.h"
#include "fj_texture.h"
#include "fj_memory.h"
#include "fj_vector.h"

#include <cfloat>

namespace fj {

static int point_light_get_sample_count(const struct Light *light);
static void point_light_get_samples(const struct Light *light,
    struct LightSample *samples, int max_samples);
static void point_light_illuminate(const struct Light *light,
    const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl);

static int grid_light_get_sample_count(const struct Light *light);
static void grid_light_get_samples(const struct Light *light,
    struct LightSample *samples, int max_samples);
static void grid_light_illuminate(const struct Light *light,
    const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl);

static int sphere_light_get_sample_count(const struct Light *light);
static void sphere_light_get_samples(const struct Light *light,
    struct LightSample *samples, int max_samples);
static void sphere_light_illuminate(const struct Light *light,
    const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl);

static int dome_light_get_sample_count(const struct Light *light);
static void dome_light_get_samples(const struct Light *light,
    struct LightSample *samples, int max_samples);
static void dome_light_illuminate(const struct Light *light,
    const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl);
static int dome_light_preprocess(struct Light *light);

static int no_preprocess(struct Light *light);

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
  dome_samples_(NULL),

  GetSampleCount_(NULL),
  GetSamples_(NULL),
  Illuminate_(NULL),
  Preprocess_(NULL)
{
  SetLightType(LGT_POINT);
}

Light::~Light()
{
  if (dome_samples_ != NULL) {
    FJ_MEM_FREE(dome_samples_);
  }
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
  color_.r = r;
  color_.g = g;
  color_.b = b;
}

void Light::SetIntensity(float intensity)
{
  intensity_ = intensity;
  /* TODO temp */
  sample_intensity_ = intensity_ / sample_count_;
}

void Light::SetSampleCount(int sample_count)
{
  sample_count_ = Max(sample_count, 1);
  /* TODO temp */
  sample_intensity_ = intensity_ / sample_count_;
}

void Light::SetDoubleSided(bool on_or_off)
{
  double_sided_ = on_or_off;
}

void Light::SetEnvironmentMap(struct Texture *texture)
{
  environment_map_ = texture;
}

void Light::SetTranslate(double tx, double ty, double tz, double time)
{
  XfmPushTranslateSample(&transform_samples_, tx, ty, tz, time);
}

void Light::SetRotate(double rx, double ry, double rz, double time)
{
  XfmPushRotateSample(&transform_samples_, rx, ry, rz, time);
}

void Light::SetScale(double sx, double sy, double sz, double time)
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

struct Light *LgtNew(int light_type)
{
  Light *light = new Light();
  light->SetLightType(light_type);
  return light;
}

void LgtFree(struct Light *light)
{
  delete light;
}

void LgtSetColor(struct Light *light, float r, float g, float b)
{
  light->color_.r = r;
  light->color_.g = g;
  light->color_.b = b;
}

void LgtSetIntensity(struct Light *light, double intensity)
{
  light->intensity_ = intensity;
  /* TODO temp */
  light->sample_intensity_ = light->intensity_ / light->sample_count_;
}

void LgtSetSampleCount(struct Light *light, int sample_count)
{
  light->sample_count_ = Max(sample_count, 1);
  /* TODO temp */
  light->sample_intensity_ = light->intensity_ / light->sample_count_;
}

void LgtSetDoubleSided(struct Light *light, int on_or_off)
{
  light->double_sided_ = (on_or_off != 0);
}

void LgtSetEnvironmentMap(struct Light *light, struct Texture *texture)
{
  light->environment_map_ = texture;
}

void LgtSetTranslate(struct Light *light,
    double tx, double ty, double tz, double time)
{
  XfmPushTranslateSample(&light->transform_samples_, tx, ty, tz, time);
}

void LgtSetRotate(struct Light *light,
    double rx, double ry, double rz, double time)
{
  XfmPushRotateSample(&light->transform_samples_, rx, ry, rz, time);
}

void LgtSetScale(struct Light *light,
    double sx, double sy, double sz, double time)
{
  XfmPushScaleSample(&light->transform_samples_, sx, sy, sz, time);
}

void LgtSetTransformOrder(struct Light *light, int order)
{
  XfmSetSampleTransformOrder(&light->transform_samples_, order);
}

void LgtSetRotateOrder(struct Light *light, int order)
{
  XfmSetSampleRotateOrder(&light->transform_samples_, order);
}

void LgtGetSamples(const struct Light *light,
    struct LightSample *samples, int max_samples)
{
  light->GetSamples_(light, samples, max_samples);
}

int LgtGetSampleCount(const struct Light *light)
{
  return light->GetSampleCount_(light);
}

/* TODO should have struct Light *light parameter? */
void LgtIlluminate(const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl)
{
  const struct Light *light = sample->light;
  light->Illuminate_(light, sample, Ps, Cl);
}

int LgtPreprocess(struct Light *light)
{
  return light->Preprocess_(light);
}

/* point light */
static int point_light_get_sample_count(const struct Light *light)
{
  return 1;
}

static void point_light_get_samples(const struct Light *light,
    struct LightSample *samples, int max_samples)
{
  struct Transform transform_interp;

  if (max_samples == 0)
    return;

  /* TODO time sampling */
  XfmLerpTransformSample(&light->transform_samples_, 0, &transform_interp);

  samples[0].P = transform_interp.translate;
  samples[0].N = Vector(0, 0, 0);
  samples[0].light = light;
}

static void point_light_illuminate(const struct Light *light,
    const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl)
{
  Cl->r = light->intensity_ * light->color_.r;
  Cl->g = light->intensity_ * light->color_.g;
  Cl->b = light->intensity_ * light->color_.b;
}

/* grid light */
static int grid_light_get_sample_count(const struct Light *light)
{
  return light->sample_count_;
}

static void grid_light_get_samples(const struct Light *light,
    struct LightSample *samples, int max_samples)
{
  struct Transform transform_interp;
  int nsamples = LgtGetSampleCount(light);
  struct Vector N_sample(0, 1, 0);
  int i;

  /* TODO time sampling */
  XfmLerpTransformSample(&light->transform_samples_, 0, &transform_interp);

  XfmTransformVector(&transform_interp, &N_sample);
  Normalize(&N_sample);

  nsamples = Min(nsamples, max_samples);
  for (i = 0; i < nsamples; i++) {
    struct XorShift *mutable_xr = (struct XorShift *) &light->xr_;
    const double x = (XorNextFloat01(mutable_xr) - .5);
    const double z = (XorNextFloat01(mutable_xr) - .5);
    struct Vector P_sample;
    P_sample.x = x;
    P_sample.z = z;

    XfmTransformPoint(&transform_interp, &P_sample);

    samples[i].P = P_sample;
    samples[i].N = N_sample;
    samples[i].light = light;
  }
}

static void grid_light_illuminate(const struct Light *light,
    const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl)
{
  struct Vector Ln;
  double dot = 0;

  Ln.x = Ps->x - sample->P.x;
  Ln.y = Ps->y - sample->P.y;
  Ln.z = Ps->z - sample->P.z;

  Normalize(&Ln);

  dot = Dot(Ln, sample->N);
  if (light->double_sided_) {
    dot = Abs(dot);
  } else {
    dot = Max(dot, 0.);
  }

  Cl->r = light->sample_intensity_ * light->color_.r;
  Cl->g = light->sample_intensity_ * light->color_.g;
  Cl->b = light->sample_intensity_ * light->color_.b;

  Cl->r *= dot;
  Cl->g *= dot;
  Cl->b *= dot;
}

/* sphere light */
static int sphere_light_get_sample_count(const struct Light *light)
{
  return light->sample_count_;
}

static void sphere_light_get_samples(const struct Light *light,
    struct LightSample *samples, int max_samples)
{
  struct Transform transform_interp;
  int nsamples = LgtGetSampleCount(light);
  int i;

  /* TODO time sampling */
  XfmLerpTransformSample(&light->transform_samples_, 0, &transform_interp);

  nsamples = Min(nsamples, max_samples);
  for (i = 0; i < nsamples; i++) {
    struct XorShift *mutable_xr = (struct XorShift *) &light->xr_;
    struct Vector P_sample;
    struct Vector N_sample;

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

static void sphere_light_illuminate(const struct Light *light,
    const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl)
{
  struct Vector Ln;
  double dot = 0;

  Ln.x = Ps->x - sample->P.x;
  Ln.y = Ps->y - sample->P.y;
  Ln.z = Ps->z - sample->P.z;

  Normalize(&Ln);

  dot = Dot(Ln, sample->N);

  if (dot > 0) {
    Cl->r = light->sample_intensity_ * light->color_.r;
    Cl->g = light->sample_intensity_ * light->color_.g;
    Cl->b = light->sample_intensity_ * light->color_.b;
  } else {
    Cl->r = 0;
    Cl->g = 0;
    Cl->b = 0;
  }
}

/* dome light */
static int dome_light_get_sample_count(const struct Light *light)
{
  return light->sample_count_;
}

static void dome_light_get_samples(const struct Light *light,
    struct LightSample *samples, int max_samples)
{
  struct Transform transform_interp;
  int nsamples = LgtGetSampleCount(light);
  int i;

  /* TODO time sampling */
  XfmLerpTransformSample(&light->transform_samples_, 0, &transform_interp);

  nsamples = Min(nsamples, max_samples);
  for (i = 0; i < nsamples; i++) {
    const struct DomeSample *dome_sample = &light->dome_samples_[i];
    struct Vector P_sample;
    struct Vector N_sample;

    P_sample.x = dome_sample->dir.x * FLT_MAX;
    P_sample.y = dome_sample->dir.y * FLT_MAX;
    P_sample.z = dome_sample->dir.z * FLT_MAX;
    N_sample.x = -1 * dome_sample->dir.x;
    N_sample.y = -1 * dome_sample->dir.y;
    N_sample.z = -1 * dome_sample->dir.z;

    /* TODO cancel translate and scale */
    XfmTransformPoint(&transform_interp, &P_sample);
    XfmTransformVector(&transform_interp, &N_sample);

    samples[i].P = P_sample;
    samples[i].N = N_sample;
    samples[i].color = dome_sample->color;
    samples[i].light = light;
  }
}

static void dome_light_illuminate(const struct Light *light,
    const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl)
{
  Cl->r = light->sample_intensity_ * sample->color.r;
  Cl->g = light->sample_intensity_ * sample->color.g;
  Cl->b = light->sample_intensity_ * sample->color.b;
}

static int dome_light_preprocess(struct Light *light)
{
  const int NSAMPLES = LgtGetSampleCount(light);
  int XRES = 200;
  int YRES = 100;
  int i;

  if (light->dome_samples_ != NULL) {
    FJ_MEM_FREE(light->dome_samples_);
  }

  light->dome_samples_ = FJ_MEM_ALLOC_ARRAY(struct DomeSample, NSAMPLES);
  for (i = 0; i < NSAMPLES; i++) {
    struct DomeSample *sample = &light->dome_samples_[i];
    sample->uv.u = 1./NSAMPLES;
    sample->uv.v = 1./NSAMPLES;
    sample->color.r = 1;
    sample->color.g = .63;
    sample->color.b = .63;
    sample->dir = Vector(1./NSAMPLES, 1, 1./NSAMPLES);
    Normalize(&sample->dir);
  }

  if (light->environment_map_ == NULL) {
    /* TODO should be an error? */
    return 0;
  }

  XRES = light->environment_map_->GetWidth();
  YRES = light->environment_map_->GetHeight();
  /* TODO parameteraize */
  XRES /= 8;
  YRES /= 8;

  if (0) {
    ImportanceSampling(light->environment_map_, 0,
        XRES, YRES,
        light->dome_samples_, NSAMPLES);
  } else if (1) {
    StratifiedImportanceSampling(light->environment_map_, 0,
        XRES, YRES,
        light->dome_samples_, NSAMPLES);
  } else {
    StructuredImportanceSampling(light->environment_map_, 0,
        XRES, YRES,
        light->dome_samples_, NSAMPLES);
  }

  return 0;
}

static int no_preprocess(struct Light *light)
{
  /* do nothing */
  return 0;
}

} // namespace xxx
