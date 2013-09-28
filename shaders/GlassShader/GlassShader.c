/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_shader.h"
#include "fj_memory.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_numeric.h"

#include <string.h>
#include <stdio.h>
#include <float.h>

struct GlassShader {
  struct Color diffuse;
  struct Color specular;
  struct Color ambient;

  struct Color filter_color;

  float roughness;
  float ior;

  int do_color_filter;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
    const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "GlassShader";

static const struct ShaderFunctionTable MyFunctionTable = {
  MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);
static int set_specular(void *self, const struct PropertyValue *value);
static int set_ambient(void *self, const struct PropertyValue *value);
static int set_filter_color(void *self, const struct PropertyValue *value);
static int set_roughness(void *self, const struct PropertyValue *value);
static int set_ior(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
  {PROP_VECTOR3, "diffuse",      set_diffuse},
  {PROP_VECTOR3, "specular",     set_specular},
  {PROP_VECTOR3, "ambient",      set_ambient},
  {PROP_VECTOR3, "filter_color", set_filter_color},
  {PROP_SCALAR,  "roughness",    set_roughness},
  {PROP_SCALAR,  "ior",          set_ior},
  {PROP_NONE,    NULL,           NULL}
};

static const struct MetaInfo MyMetainfo[] = {
  {"help", "A glass shader."},
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
  struct GlassShader *glass = NULL;

  glass = FJ_MEM_ALLOC(struct GlassShader);
  if (glass == NULL)
    return NULL;

  ColSet(&glass->diffuse, 0, 0, 0);
  ColSet(&glass->specular, 1, 1, 1);
  ColSet(&glass->ambient, 1, 1, 1);
  ColSet(&glass->filter_color, 1, 1, 1);
  glass->roughness = .1;
  glass->ior = 1.4;

  glass->do_color_filter = 0;

  return glass;
}

static void MyFree(void *self)
{
  struct GlassShader *glass = (struct GlassShader *) self;
  if (glass == NULL)
    return;
  FJ_MEM_FREE(glass);
}

static void MyEvaluate(const void *self, const struct TraceContext *cxt,
    const struct SurfaceInput *in, struct SurfaceOutput *out)
{
  const struct GlassShader *glass = (struct GlassShader *) self;
  struct TraceContext refl_cxt;
  struct TraceContext refr_cxt;
  struct Vector T = {0, 0, 0};
  struct Vector R = {0, 0, 0};
  struct Color4 C_refl = {0, 0, 0, 0};
  struct Color4 C_refr = {0, 0, 0, 0};
  double Kt = 0, Kr = 0;
  double t_hit = FLT_MAX;

  /* Cs */
  ColSet(&out->Cs, 0, 0, 0);

  Kr = SlFresnel(&in->I, &in->N, 1/glass->ior);
  Kt = 1 - Kr;

  /* reflect */
  refl_cxt = SlReflectContext(cxt, in->shaded_object);
  SlReflect(&in->I, &in->N, &R);
  VEC3_NORMALIZE(&R);
  /* TODO fix hard-coded trace distance */
  SlTrace(&refl_cxt, &in->P, &R, .0001, 1000, &C_refl, &t_hit);
  out->Cs.r += Kr * C_refl.r;
  out->Cs.g += Kr * C_refl.g;
  out->Cs.b += Kr * C_refl.b;

  /* refract */
  refr_cxt = SlRefractContext(cxt, in->shaded_object);
  SlRefract(&in->I, &in->N, 1/glass->ior, &T);
  VEC3_NORMALIZE(&T);
  SlTrace(&refr_cxt, &in->P, &T, .0001, 1000, &C_refr, &t_hit);

  if (glass->do_color_filter && VEC3_DOT(&in->I, &in->N) < 0) {
    C_refr.r *= pow(glass->filter_color.r, t_hit);
    C_refr.g *= pow(glass->filter_color.g, t_hit);
    C_refr.b *= pow(glass->filter_color.b, t_hit);
  }

  out->Cs.r += Kt * C_refr.r;
  out->Cs.g += Kt * C_refr.g;
  out->Cs.b += Kt * C_refr.b;

  out->Os = 1;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
  struct GlassShader *glass = (struct GlassShader *) self;
  struct Color diffuse = {0, 0, 0};

  diffuse.r = MAX(0, value->vector[0]);
  diffuse.g = MAX(0, value->vector[1]);
  diffuse.b = MAX(0, value->vector[2]);
  glass->diffuse = diffuse;

  return 0;
}

static int set_specular(void *self, const struct PropertyValue *value)
{
  struct GlassShader *glass = (struct GlassShader *) self;
  struct Color specular = {0, 0, 0};

  specular.r = MAX(0, value->vector[0]);
  specular.g = MAX(0, value->vector[1]);
  specular.b = MAX(0, value->vector[2]);
  glass->specular = specular;

  return 0;
}

static int set_ambient(void *self, const struct PropertyValue *value)
{
  struct GlassShader *glass = (struct GlassShader *) self;
  struct Color ambient = {0, 0, 0};

  ambient.r = MAX(0, value->vector[0]);
  ambient.g = MAX(0, value->vector[1]);
  ambient.b = MAX(0, value->vector[2]);
  glass->ambient = ambient;

  return 0;
}

static int set_filter_color(void *self, const struct PropertyValue *value)
{
  struct GlassShader *glass = (struct GlassShader *) self;
  struct Color filter_color = {0, 0, 0};

  filter_color.r = MAX(.001, value->vector[0]);
  filter_color.g = MAX(.001, value->vector[1]);
  filter_color.b = MAX(.001, value->vector[2]);
  glass->filter_color = filter_color;

  if (glass->filter_color.r == 1 &&
    glass->filter_color.g == 1 &&
    glass->filter_color.b == 1) {
    glass->do_color_filter = 0;
  }
  else {
    glass->do_color_filter = 1;
  }

  return 0;
}

static int set_roughness(void *self, const struct PropertyValue *value)
{
  struct GlassShader *glass = (struct GlassShader *) self;
  float roughness = value->vector[0];

  roughness = MAX(0, roughness);
  glass->roughness = roughness;

  return 0;
}

static int set_ior(void *self, const struct PropertyValue *value)
{
  struct GlassShader *glass = (struct GlassShader *) self;
  float ior = value->vector[0];

  ior = MAX(0, ior);
  glass->ior = ior;

  return 0;
}

