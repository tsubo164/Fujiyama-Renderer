/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_shader.h"
#include "fj_numeric.h"
#include "fj_memory.h"
#include "fj_vector.h"
#include "fj_color.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <float.h>

struct HairShader {
  struct Color diffuse;
  struct Color specular;
  struct Color ambient;
  float roughness;

  struct Color reflect;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
    const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "HairShader";
static const struct ShaderFunctionTable MyFunctionTable = {
  MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);
static int set_specular(void *self, const struct PropertyValue *value);
static int set_ambient(void *self, const struct PropertyValue *value);
static int set_roughness(void *self, const struct PropertyValue *value);
static int set_reflect(void *self, const struct PropertyValue *value);

/* hair shading implementations */
static float kajiya_diffuse(const struct Vector *tangent, const struct Vector *Ln);
static float kajiya_specular(const struct Vector *tangent,
    const struct Vector *Ln, const struct Vector *I);

static const struct Property MyProperties[] = {
  {PROP_VECTOR3, "diffuse",   {1, 1, 1, 0}, set_diffuse},
  {PROP_VECTOR3, "specular",  {1, 1, 1, 0},    set_specular},
  {PROP_VECTOR3, "ambient",   {1, 1, 1, 0},    set_ambient},
  {PROP_SCALAR,  "roughness", {.1, 0, 0, 0},   set_roughness},
  {PROP_VECTOR3, "reflect",   {1, 1, 1, 0},    set_reflect},
  {PROP_NONE,    NULL,        {0, 0, 0, 0},    NULL}
};

static const struct MetaInfo MyMetainfo[] = {
  {"help", "A hair shader."},
  {"plugin_type", "Shader"},
  {NULL, NULL}
};

extern "C" {
int Initialize(struct PluginInfo *info)
{
  return PlgSetupInfo(info,
      PLUGIN_API_VERSION,
      SHADER_PLUGIN_TYPE,
      MyPluginName,
      MyNew,
      MyFree,
      &MyFunctionTable,
      MyProperties,
      MyMetainfo);
}
} // extern "C"

static void *MyNew(void)
{
  struct HairShader *hair = FJ_MEM_ALLOC(struct HairShader);

  if (hair == NULL)
    return NULL;

  PropSetAllDefaultValues(hair, MyProperties);

  return hair;
}

static void MyFree(void *self)
{
  struct HairShader *hair = (struct HairShader *) self;
  if (hair == NULL)
    return;
  FJ_MEM_FREE(hair);
}

static void MyEvaluate(const void *self, const struct TraceContext *cxt,
    const struct SurfaceInput *in, struct SurfaceOutput *out)
{
  const struct HairShader *hair = (struct HairShader *) self;
  int i = 0;

  struct LightSample *samples = NULL;
  const int nsamples = SlGetLightSampleCount(in);

  /* allocate samples */
  samples = SlNewLightSamples(in);

  ColSet(&out->Cs, 0, 0, 0);

  for (i = 0; i < nsamples; i++) {
    struct LightOutput Lout = {{0}};
    struct Vector tangent = {0, 0, 0};
    float diff = 0;
    float spec = 0;

    SlIlluminance(cxt, &samples[i], &in->P, &in->N, N_PI, in, &Lout);

    tangent = in->dPdv;
    VEC3_NORMALIZE(&tangent);

    diff = kajiya_diffuse(&tangent, &Lout.Ln);
    spec = kajiya_specular(&tangent, &Lout.Ln, &in->I);

    out->Cs.r += (in->Cd.r * hair->diffuse.r * diff + spec) * Lout.Cl.r;
    out->Cs.g += (in->Cd.g * hair->diffuse.g * diff + spec) * Lout.Cl.g;
    out->Cs.b += (in->Cd.b * hair->diffuse.b * diff + spec) * Lout.Cl.b;
  }

  /* FJ_MEM_FREE samples */
  SlFreeLightSamples(samples);

  out->Os = 1;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
  struct HairShader *hair = (struct HairShader *) self;
  struct Color diffuse = {0, 0, 0};

  diffuse.r = MAX(0, value->vector[0]);
  diffuse.g = MAX(0, value->vector[1]);
  diffuse.b = MAX(0, value->vector[2]);
  hair->diffuse = diffuse;

  return 0;
}

static int set_specular(void *self, const struct PropertyValue *value)
{
  struct HairShader *hair = (struct HairShader *) self;
  struct Color specular = {0, 0, 0};

  specular.r = MAX(0, value->vector[0]);
  specular.g = MAX(0, value->vector[1]);
  specular.b = MAX(0, value->vector[2]);
  hair->specular = specular;

  return 0;
}

static int set_ambient(void *self, const struct PropertyValue *value)
{
  struct HairShader *hair = (struct HairShader *) self;
  struct Color ambient = {0, 0, 0};

  ambient.r = MAX(0, value->vector[0]);
  ambient.g = MAX(0, value->vector[1]);
  ambient.b = MAX(0, value->vector[2]);
  hair->ambient = ambient;

  return 0;
}

static int set_roughness(void *self, const struct PropertyValue *value)
{
  struct HairShader *hair = (struct HairShader *) self;
  float roughness = value->vector[0];

  roughness = MAX(0, roughness);
  hair->roughness = roughness;

  return 0;
}

static int set_reflect(void *self, const struct PropertyValue *value)
{
  struct HairShader *hair = (struct HairShader *) self;
  struct Color reflect = {0, 0, 0};

  reflect.r = MAX(0, value->vector[0]);
  reflect.g = MAX(0, value->vector[1]);
  reflect.b = MAX(0, value->vector[2]);
  hair->reflect = reflect;

  return 0;
}

static float kajiya_diffuse(const struct Vector *tangent, const struct Vector *Ln)
{
  const float TL = VEC3_DOT(tangent, Ln);
  const float diff = sqrt(1-TL*TL);

  return diff;
}

static float kajiya_specular(const struct Vector *tangent,
    const struct Vector *Ln, const struct Vector *I)
{
  float spec = 0;
  const float roughness = .05;
  const float TL = VEC3_DOT(tangent, Ln);
  const float TI = VEC3_DOT(tangent, I);

  assert(-1 <= TL && TL <= 1);
  assert(-1 <= TI && TI <= 1);

  spec = sqrt(1-TL*TL) * sqrt(1-TI*TI) + TL*TI;
  spec = pow(spec, 1/roughness);

  return spec;
}

