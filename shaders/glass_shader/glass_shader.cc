// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_color.h"

#include <cstring>
#include <cstdio>
#include <cfloat>

using namespace fj;

class GlassShader {
public:
  GlassShader() {}
  ~GlassShader() {}

public:
  Color diffuse;
  Color specular;
  Color ambient;

  Color filter_color;

  float roughness;
  float ior;

  int do_color_filter;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out);

static const char MyPluginName[] = "GlassShader";

static const ShaderFunctionTable MyFunctionTable = {
  MyEvaluate
};

static int set_diffuse(void *self, const PropertyValue *value);
static int set_specular(void *self, const PropertyValue *value);
static int set_ambient(void *self, const PropertyValue *value);
static int set_filter_color(void *self, const PropertyValue *value);
static int set_roughness(void *self, const PropertyValue *value);
static int set_ior(void *self, const PropertyValue *value);

static const Property MyProperties[] = {
  {PROP_VECTOR3, "diffuse",      {0, 0, 0, 0},   set_diffuse},
  {PROP_VECTOR3, "specular",     {1, 1, 1, 0},   set_specular},
  {PROP_VECTOR3, "ambient",      {1, 1, 1, 0},   set_ambient},
  {PROP_VECTOR3, "filter_color", {1, 1, 1, 0},   set_filter_color},
  {PROP_SCALAR,  "roughness",    {.1, 0, 0, 0},  set_roughness},
  {PROP_SCALAR,  "ior",          {1.4, 0, 0, 0}, set_ior},
  {PROP_NONE,    NULL,           {0, 0, 0, 0},   NULL}
};

static const MetaInfo MyMetainfo[] = {
  {"help", "A glass shader."},
  {"plugin_type", "Shader"},
  {NULL, NULL}
};

extern "C" {
FJ_PLUGIN_API int Initialize(PluginInfo *info)
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
  GlassShader *glass = new GlassShader();

  PropSetAllDefaultValues(glass, MyProperties);

  return glass;
}

static void MyFree(void *self)
{
  GlassShader *glass = (GlassShader *) self;
  if (glass == NULL)
    return;
  delete glass;
}

static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out)
{
  const GlassShader *glass = (GlassShader *) self;
  TraceContext refl_cxt;
  TraceContext refr_cxt;
  Vector T;
  Vector R;
  Color4 C_refl;
  Color4 C_refr;
  double Kt = 0, Kr = 0;
  double t_hit = FLT_MAX;

  // Cs
  out->Cs = Color();

  Kr = SlFresnel(&in->I, &in->N, 1/glass->ior);
  Kt = 1 - Kr;

  // reflect
  refl_cxt = SlReflectContext(cxt, in->shaded_object);
  SlReflect(&in->I, &in->N, &R);
  Normalize(&R);
  // TODO fix hard-coded trace distance
  SlTrace(&refl_cxt, &in->P, &R, .0001, 1000, &C_refl, &t_hit);
  out->Cs.r += Kr * C_refl.r;
  out->Cs.g += Kr * C_refl.g;
  out->Cs.b += Kr * C_refl.b;

  // refract
  refr_cxt = SlRefractContext(cxt, in->shaded_object);
  SlRefract(&in->I, &in->N, 1/glass->ior, &T);
  Normalize(&T);
  SlTrace(&refr_cxt, &in->P, &T, .0001, 1000, &C_refr, &t_hit);

  if (glass->do_color_filter && Dot(in->I, in->N) < 0) {
    C_refr.r *= pow(glass->filter_color.r, t_hit);
    C_refr.g *= pow(glass->filter_color.g, t_hit);
    C_refr.b *= pow(glass->filter_color.b, t_hit);
  }

  out->Cs.r += Kt * C_refr.r;
  out->Cs.g += Kt * C_refr.g;
  out->Cs.b += Kt * C_refr.b;

  out->Os = 1;
}

static int set_diffuse(void *self, const PropertyValue *value)
{
  GlassShader *glass = (GlassShader *) self;
  Color diffuse;

  diffuse.r = Max(0, value->vector[0]);
  diffuse.g = Max(0, value->vector[1]);
  diffuse.b = Max(0, value->vector[2]);
  glass->diffuse = diffuse;

  return 0;
}

static int set_specular(void *self, const PropertyValue *value)
{
  GlassShader *glass = (GlassShader *) self;
  Color specular;

  specular.r = Max(0, value->vector[0]);
  specular.g = Max(0, value->vector[1]);
  specular.b = Max(0, value->vector[2]);
  glass->specular = specular;

  return 0;
}

static int set_ambient(void *self, const PropertyValue *value)
{
  GlassShader *glass = (GlassShader *) self;
  Color ambient;

  ambient.r = Max(0, value->vector[0]);
  ambient.g = Max(0, value->vector[1]);
  ambient.b = Max(0, value->vector[2]);
  glass->ambient = ambient;

  return 0;
}

static int set_filter_color(void *self, const PropertyValue *value)
{
  GlassShader *glass = (GlassShader *) self;
  Color filter_color;

  filter_color.r = Max(.001, value->vector[0]);
  filter_color.g = Max(.001, value->vector[1]);
  filter_color.b = Max(.001, value->vector[2]);
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

static int set_roughness(void *self, const PropertyValue *value)
{
  GlassShader *glass = (GlassShader *) self;
  float roughness = value->vector[0];

  roughness = Max(0, roughness);
  glass->roughness = roughness;

  return 0;
}

static int set_ior(void *self, const PropertyValue *value)
{
  GlassShader *glass = (GlassShader *) self;
  float ior = value->vector[0];

  ior = Max(0, ior);
  glass->ior = ior;

  return 0;
}
