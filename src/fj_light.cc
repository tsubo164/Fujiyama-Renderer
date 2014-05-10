/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_light.h"
#include "fj_importance_sampling.h"
#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_numeric.h"
#include "fj_texture.h"
#include "fj_memory.h"
#include "fj_random.h"
#include "fj_vector.h"

#include <float.h>

struct Light {
  struct Color color;
  float intensity;

  /* transformation properties */
  struct TransformSampleList transform_samples;

  /* rng */
  struct XorShift xr;

  int type;
  int double_sided;
  int sample_count;
  float sample_intensity;

  struct Texture *environment_map;
  /* TODO tmp solution for dome light data */
  struct DomeSample *dome_samples;

  /* functions */
  int (*GetSampleCount)(const struct Light *light);
  void (*GetSamples)(const struct Light *light,
      struct LightSample *samples, int max_samples);
  void (*Illuminate)(const struct Light *light,
      const struct LightSample *sample,
      const struct Vector *Ps, struct Color *Cl);
  int (*Preprocess)(struct Light *light);
};

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

struct Light *LgtNew(int light_type)
{
  struct Light *light = FJ_MEM_ALLOC(struct Light);
  if (light == NULL)
    return NULL;

  XfmInitTransformSampleList(&light->transform_samples);

  light->color.r = 1;
  light->color.g = 1;
  light->color.b = 1;
  light->intensity = 1;

  light->type = light_type;
  light->double_sided = 0;
  light->sample_count = 16;
  light->sample_intensity = light->intensity / light->sample_count;

  light->environment_map = NULL;
  light->dome_samples = NULL;
  XorInit(&light->xr);

  switch (light->type) {
  case LGT_POINT:
    light->GetSampleCount = point_light_get_sample_count;
    light->GetSamples     = point_light_get_samples;
    light->Illuminate     = point_light_illuminate;
    light->Preprocess     = no_preprocess;
    break;
  case LGT_GRID:
    light->GetSampleCount = grid_light_get_sample_count;
    light->GetSamples     = grid_light_get_samples;
    light->Illuminate     = grid_light_illuminate;
    light->Preprocess     = no_preprocess;
    break;
  case LGT_SPHERE:
    light->GetSampleCount = sphere_light_get_sample_count;
    light->GetSamples     = sphere_light_get_samples;
    light->Illuminate     = sphere_light_illuminate;
    light->Preprocess     = no_preprocess;
    break;
  case LGT_DOME:
    light->GetSampleCount = dome_light_get_sample_count;
    light->GetSamples     = dome_light_get_samples;
    light->Illuminate     = dome_light_illuminate;
    light->Preprocess     = dome_light_preprocess;
    break;
  default:
    /* TODO should abort()? */
    light->type = LGT_POINT;
    light->GetSampleCount = point_light_get_sample_count;
    light->GetSamples     = point_light_get_samples;
    light->Illuminate     = point_light_illuminate;
    light->Preprocess     = no_preprocess;
    break;
  }

  return light;
}

void LgtFree(struct Light *light)
{
  if (light == NULL)
    return;

  if (light->dome_samples != NULL)
    FJ_MEM_FREE(light->dome_samples);

  FJ_MEM_FREE(light);
}

void LgtSetColor(struct Light *light, float r, float g, float b)
{
  light->color.r = r;
  light->color.g = g;
  light->color.b = b;
}

void LgtSetIntensity(struct Light *light, double intensity)
{
  light->intensity = intensity;
  /* TODO temp */
  light->sample_intensity = light->intensity / light->sample_count;
}

void LgtSetSampleCount(struct Light *light, int sample_count)
{
  light->sample_count = MAX(sample_count, 1);
  /* TODO temp */
  light->sample_intensity = light->intensity / light->sample_count;
}

void LgtSetDoubleSided(struct Light *light, int on_or_off)
{
  light->double_sided = (on_or_off != 0);
}

void LgtSetEnvironmentMap(struct Light *light, struct Texture *texture)
{
  light->environment_map = texture;
}

void LgtSetTranslate(struct Light *light,
    double tx, double ty, double tz, double time)
{
  XfmPushTranslateSample(&light->transform_samples, tx, ty, tz, time);
}

void LgtSetRotate(struct Light *light,
    double rx, double ry, double rz, double time)
{
  XfmPushRotateSample(&light->transform_samples, rx, ry, rz, time);
}

void LgtSetScale(struct Light *light,
    double sx, double sy, double sz, double time)
{
  XfmPushScaleSample(&light->transform_samples, sx, sy, sz, time);
}

void LgtSetTransformOrder(struct Light *light, int order)
{
  XfmSetSampleTransformOrder(&light->transform_samples, order);
}

void LgtSetRotateOrder(struct Light *light, int order)
{
  XfmSetSampleRotateOrder(&light->transform_samples, order);
}

void LgtGetSamples(const struct Light *light,
    struct LightSample *samples, int max_samples)
{
  light->GetSamples(light, samples, max_samples);
}

int LgtGetSampleCount(const struct Light *light)
{
  return light->GetSampleCount(light);
}

/* TODO should have struct Light *light parameter? */
void LgtIlluminate(const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl)
{
  const struct Light *light = sample->light;
  light->Illuminate(light, sample, Ps, Cl);
}

int LgtPreprocess(struct Light *light)
{
  return light->Preprocess(light);
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
  XfmLerpTransformSample(&light->transform_samples, 0, &transform_interp);

  samples[0].P = transform_interp.translate;
  VEC3_SET(&samples[0].N, 0, 0, 0);
  samples[0].light = light;
}

static void point_light_illuminate(const struct Light *light,
    const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl)
{
  Cl->r = light->intensity * light->color.r;
  Cl->g = light->intensity * light->color.g;
  Cl->b = light->intensity * light->color.b;
}

/* grid light */
static int grid_light_get_sample_count(const struct Light *light)
{
  return light->sample_count;
}

static void grid_light_get_samples(const struct Light *light,
    struct LightSample *samples, int max_samples)
{
  struct Transform transform_interp;
  int nsamples = LgtGetSampleCount(light);
  struct Vector N_sample(0, 1, 0);
  int i;

  /* TODO time sampling */
  XfmLerpTransformSample(&light->transform_samples, 0, &transform_interp);

  XfmTransformVector(&transform_interp, &N_sample);
  VEC3_NORMALIZE(&N_sample);

  nsamples = MIN(nsamples, max_samples);
  for (i = 0; i < nsamples; i++) {
    struct XorShift *mutable_xr = (struct XorShift *) &light->xr;
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

  VEC3_NORMALIZE(&Ln);

  dot = VEC3_DOT(&Ln, &sample->N);
  if (light->double_sided) {
    dot = ABS(dot);
  } else {
    dot = MAX(dot, 0);
  }

  Cl->r = light->sample_intensity * light->color.r;
  Cl->g = light->sample_intensity * light->color.g;
  Cl->b = light->sample_intensity * light->color.b;

  Cl->r *= dot;
  Cl->g *= dot;
  Cl->b *= dot;
}

/* sphere light */
static int sphere_light_get_sample_count(const struct Light *light)
{
  return light->sample_count;
}

static void sphere_light_get_samples(const struct Light *light,
    struct LightSample *samples, int max_samples)
{
  struct Transform transform_interp;
  int nsamples = LgtGetSampleCount(light);
  int i;

  /* TODO time sampling */
  XfmLerpTransformSample(&light->transform_samples, 0, &transform_interp);

  nsamples = MIN(nsamples, max_samples);
  for (i = 0; i < nsamples; i++) {
    struct XorShift *mutable_xr = (struct XorShift *) &light->xr;
    struct Vector P_sample;
    struct Vector N_sample;

    XorHollowSphereRand(mutable_xr, &P_sample);
    N_sample = P_sample;

    XfmTransformPoint(&transform_interp, &P_sample);
    XfmTransformVector(&transform_interp, &N_sample);
    VEC3_NORMALIZE(&N_sample);

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

  VEC3_NORMALIZE(&Ln);

  dot = VEC3_DOT(&Ln, &sample->N);

  if (dot > 0) {
    Cl->r = light->sample_intensity * light->color.r;
    Cl->g = light->sample_intensity * light->color.g;
    Cl->b = light->sample_intensity * light->color.b;
  } else {
    Cl->r = 0;
    Cl->g = 0;
    Cl->b = 0;
  }
}

/* dome light */
static int dome_light_get_sample_count(const struct Light *light)
{
  return light->sample_count;
}

static void dome_light_get_samples(const struct Light *light,
    struct LightSample *samples, int max_samples)
{
  struct Transform transform_interp;
  int nsamples = LgtGetSampleCount(light);
  int i;

  /* TODO time sampling */
  XfmLerpTransformSample(&light->transform_samples, 0, &transform_interp);

  nsamples = MIN(nsamples, max_samples);
  for (i = 0; i < nsamples; i++) {
    const struct DomeSample *dome_sample = &light->dome_samples[i];
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
  Cl->r = light->sample_intensity * sample->color.r;
  Cl->g = light->sample_intensity * sample->color.g;
  Cl->b = light->sample_intensity * sample->color.b;
}

static int dome_light_preprocess(struct Light *light)
{
  const int NSAMPLES = LgtGetSampleCount(light);
  int XRES = 200;
  int YRES = 100;
  int i;

  if (light->dome_samples != NULL) {
    FJ_MEM_FREE(light->dome_samples);
  }

  light->dome_samples = FJ_MEM_ALLOC_ARRAY(struct DomeSample, NSAMPLES);
  for (i = 0; i < NSAMPLES; i++) {
    struct DomeSample *sample = &light->dome_samples[i];
    sample->uv.u = 1./NSAMPLES;
    sample->uv.v = 1./NSAMPLES;
    sample->color.r = 1;
    sample->color.g = .63;
    sample->color.b = .63;
    VEC3_SET(&sample->dir, 1./NSAMPLES, 1, 1./NSAMPLES);
    VEC3_NORMALIZE(&sample->dir);
  }

  if (light->environment_map == NULL) {
    /* TODO should be an error? */
    return 0;
  }

  XRES = TexGetWidth(light->environment_map);
  YRES = TexGetHeight(light->environment_map);
  /* TODO parameteraize */
  XRES /= 8;
  YRES /= 8;

  if (0) {
    ImportanceSampling(light->environment_map, 0,
        XRES, YRES,
        light->dome_samples, NSAMPLES);
  } else if (1) {
    StratifiedImportanceSampling(light->environment_map, 0,
        XRES, YRES,
        light->dome_samples, NSAMPLES);
  } else {
    StructuredImportanceSampling(light->environment_map, 0,
        XRES, YRES,
        light->dome_samples, NSAMPLES);
  }

  return 0;
}

static int no_preprocess(struct Light *light)
{
  /* do nothing */
  return 0;
}

