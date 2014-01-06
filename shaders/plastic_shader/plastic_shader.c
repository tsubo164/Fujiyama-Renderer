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
#include <stdio.h>
#include <float.h>

struct PlasticShader {
  struct Color diffuse;
  struct Color specular;
  struct Color ambient;
  float roughness;

  struct Color reflect;
  float ior;

  float opacity;

  int do_reflect;

  struct Texture *diffuse_map;
  struct Texture *bump_map;
  float bump_amplitude;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
    const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "PlasticShader";
static const struct ShaderFunctionTable MyFunctionTable = {
  MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);
static int set_specular(void *self, const struct PropertyValue *value);
static int set_ambient(void *self, const struct PropertyValue *value);
static int set_roughness(void *self, const struct PropertyValue *value);
static int set_reflect(void *self, const struct PropertyValue *value);
static int set_ior(void *self, const struct PropertyValue *value);
static int set_opacity(void *self, const struct PropertyValue *value);
static int set_diffuse_map(void *self, const struct PropertyValue *value);
static int set_bump_map(void *self, const struct PropertyValue *value);
static int set_bump_amplitude(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
  {PROP_VECTOR3, "diffuse",     {.8, .8, .8, 0}, set_diffuse},
  {PROP_VECTOR3, "specular",    {1, 1, 1, 0},    set_specular},
  {PROP_VECTOR3, "ambient",     {1, 1, 1, 0},    set_ambient},
  {PROP_SCALAR,  "roughness",   {.1, 0, 0, 0},   set_roughness},
  {PROP_VECTOR3, "reflect",     {1, 1, 1, 0},    set_reflect},
  {PROP_SCALAR,  "ior",         {1.4, 0, 0, 0},  set_ior},
  {PROP_SCALAR,  "opacity",     {1, 0, 0, 0},    set_opacity},
  {PROP_TEXTURE, "diffuse_map", {0, 0, 0, 0},    set_diffuse_map},
  {PROP_TEXTURE, "bump_map",    {0, 0, 0, 0},    set_bump_map},
  {PROP_SCALAR,  "bump_amplitude", {1, 0, 0, 0},    set_bump_amplitude},
  {PROP_NONE,    NULL,          {0, 0, 0, 0},    NULL}
};

static const struct MetaInfo MyMetainfo[] = {
  {"help", "A plastic shader."},
  {"plugin_type", "Shader"},
  {NULL, NULL}
};

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

static void *MyNew(void)
{
  struct PlasticShader *plastic = FJ_MEM_ALLOC(struct PlasticShader);

  if (plastic == NULL)
    return NULL;

  PropSetAllDefaultValues(plastic, MyProperties);

  return plastic;
}

static void MyFree(void *self)
{
  struct PlasticShader *plastic = (struct PlasticShader *) self;
  if (plastic == NULL)
    return;
  FJ_MEM_FREE(plastic);
}

static void MyEvaluate(const void *self, const struct TraceContext *cxt,
    const struct SurfaceInput *in, struct SurfaceOutput *out)
{
  const struct PlasticShader *plastic = (struct PlasticShader *) self;
  struct Color diff = {0, 0, 0};
  struct Color spec = {0, 0, 0};
  struct Color4 diff_map = {1, 1, 1, 1};
  struct Vector Nf = {0, 0, 0};
  int i = 0;

  struct LightSample *samples = NULL;
  const int nsamples = SlGetLightSampleCount(in);

  SlFaceforward(&in->I, &in->N, &Nf);

  /* bump map */
  if (plastic->bump_map != NULL) {
    struct Vector N_bump = {0, 0, 0};
    SlBumpMapping(plastic->bump_map,
        &in->dPdu, &in->dPdv,
        &in->uv, plastic->bump_amplitude,
        &Nf, &N_bump);
    Nf = N_bump;
  }

  /* allocate samples */
  samples = SlNewLightSamples(in);

  for (i = 0; i < nsamples; i++) {
    struct LightOutput Lout;
    float Kd = 0;
    SlIlluminance(cxt, &samples[i], &in->P, &Nf, N_PI_2, in, &Lout);
    /* spec */
    /*
    Ks = SlPhong(in->I, Nf, Ln, .05);
    */

    /* diff */
    Kd = VEC3_DOT(&Nf, &Lout.Ln);
    Kd = MAX(0, Kd);
    diff.r += Kd * Lout.Cl.r;
    diff.g += Kd * Lout.Cl.g;
    diff.b += Kd * Lout.Cl.b;
  }

  /* free samples */
  SlFreeLightSamples(samples);

  /* diffuse map */
  if (plastic->diffuse_map != NULL) {
    TexLookup(plastic->diffuse_map, in->uv.u, in->uv.v, &diff_map);
  }

  /* Cs */
  out->Cs.r = diff.r * plastic->diffuse.r * diff_map.r + spec.r;
  out->Cs.g = diff.g * plastic->diffuse.g * diff_map.g + spec.g;
  out->Cs.b = diff.b * plastic->diffuse.b * diff_map.b + spec.b;

  /* reflect */
  if (plastic->do_reflect) {
    struct Color4 C_refl = {0, 0, 0, 0};
    struct Vector R = {0, 0, 0};
    double t_hit = FLT_MAX;
    double Kr = 0;

    const struct TraceContext refl_cxt = SlReflectContext(cxt, in->shaded_object);

    SlReflect(&in->I, &Nf, &R);
    VEC3_NORMALIZE(&R);
    SlTrace(&refl_cxt, &in->P, &R, .001, 1000, &C_refl, &t_hit);

    Kr = SlFresnel(&in->I, &Nf, 1/plastic->ior);
    out->Cs.r += Kr * C_refl.r * plastic->reflect.r;
    out->Cs.g += Kr * C_refl.g * plastic->reflect.g;
    out->Cs.b += Kr * C_refl.b * plastic->reflect.b;
  }

  out->Os = 1;
  out->Os = plastic->opacity;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
  struct PlasticShader *plastic = (struct PlasticShader *) self;
  struct Color diffuse;

  diffuse.r = MAX(0, value->vector[0]);
  diffuse.g = MAX(0, value->vector[1]);
  diffuse.b = MAX(0, value->vector[2]);
  plastic->diffuse = diffuse;

  return 0;
}

static int set_specular(void *self, const struct PropertyValue *value)
{
  struct PlasticShader *plastic = (struct PlasticShader *) self;
  struct Color specular;

  specular.r = MAX(0, value->vector[0]);
  specular.g = MAX(0, value->vector[1]);
  specular.b = MAX(0, value->vector[2]);
  plastic->specular = specular;

  return 0;
}

static int set_ambient(void *self, const struct PropertyValue *value)
{
  struct PlasticShader *plastic = (struct PlasticShader *) self;
  struct Color ambient;

  ambient.r = MAX(0, value->vector[0]);
  ambient.g = MAX(0, value->vector[1]);
  ambient.b = MAX(0, value->vector[2]);
  plastic->ambient = ambient;

  return 0;
}

static int set_roughness(void *self, const struct PropertyValue *value)
{
  struct PlasticShader *plastic = (struct PlasticShader *) self;
  float roughness = value->vector[0];

  roughness = MAX(0, roughness);
  plastic->roughness = roughness;

  return 0;
}

static int set_reflect(void *self, const struct PropertyValue *value)
{
  struct PlasticShader *plastic = (struct PlasticShader *) self;
  struct Color reflect;

  reflect.r = MAX(0, value->vector[0]);
  reflect.g = MAX(0, value->vector[1]);
  reflect.b = MAX(0, value->vector[2]);
  plastic->reflect = reflect;

  if (plastic->reflect.r > 0 ||
    plastic->reflect.g > 0 ||
    plastic->reflect.b > 0 ) {
    plastic->do_reflect = 1;
  }
  else {
    plastic->do_reflect = 0;
  }

  return 0;
}

static int set_ior(void *self, const struct PropertyValue *value)
{
  struct PlasticShader *plastic = (struct PlasticShader *) self;
  float ior = value->vector[0];

  ior = MAX(.001, ior);
  plastic->ior = ior;

  return 0;
}

static int set_opacity(void *self, const struct PropertyValue *value)
{
  struct PlasticShader *plastic = (struct PlasticShader *) self;
  float opacity = value->vector[0];

  opacity = CLAMP(opacity, 0, 1);
  plastic->opacity = opacity;

  return 0;
}

static int set_diffuse_map(void *self, const struct PropertyValue *value)
{
  struct PlasticShader *plastic = (struct PlasticShader *) self;

  plastic->diffuse_map = value->texture;

  return 0;
}

static int set_bump_map(void *self, const struct PropertyValue *value)
{
  struct PlasticShader *plastic = (struct PlasticShader *) self;

  plastic->bump_map = value->texture;

  return 0;
}

static int set_bump_amplitude(void *self, const struct PropertyValue *value)
{
  struct PlasticShader *plastic = (struct PlasticShader *) self;
  const float amp = value->vector[0];

  plastic->bump_amplitude = amp;

  return 0;
}
