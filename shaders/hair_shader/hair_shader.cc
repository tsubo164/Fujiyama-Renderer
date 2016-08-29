// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_color.h"

#include <cstring>
#include <cassert>
#include <cstdio>
#include <cfloat>

using namespace fj;

class HairShader {
public:
  HairShader() {}
  ~HairShader() {}

public:
  Color diffuse;
  Color specular;
  Color ambient;
  float roughness;

  Color reflect;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out);

static const char MyPluginName[] = "HairShader";
static const ShaderFunctionTable MyFunctionTable = {
  MyEvaluate
};

static int set_diffuse(void *self, const PropertyValue *value);
static int set_specular(void *self, const PropertyValue *value);
static int set_ambient(void *self, const PropertyValue *value);
static int set_roughness(void *self, const PropertyValue *value);
static int set_reflect(void *self, const PropertyValue *value);

// hair shading implementations
static float kajiya_diffuse(const Vector *tangent, const Vector *Ln);
static float kajiya_specular(const Vector *tangent,
    const Vector *Ln, const Vector *I);

static const Property MyProperties[] = {
  {PROP_VECTOR3, "diffuse",   {1, 1, 1, 0}, set_diffuse},
  {PROP_VECTOR3, "specular",  {1, 1, 1, 0},    set_specular},
  {PROP_VECTOR3, "ambient",   {1, 1, 1, 0},    set_ambient},
  {PROP_SCALAR,  "roughness", {.1, 0, 0, 0},   set_roughness},
  {PROP_VECTOR3, "reflect",   {1, 1, 1, 0},    set_reflect},
  {PROP_NONE,    NULL,        {0, 0, 0, 0},    NULL}
};

static const MetaInfo MyMetainfo[] = {
  {"help", "A hair shader."},
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
  HairShader *hair = new HairShader();

  PropSetAllDefaultValues(hair, MyProperties);

  return hair;
}

static void MyFree(void *self)
{
  HairShader *hair = (HairShader *) self;
  if (hair == NULL)
    return;
  delete hair;
}

static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out)
{
  const HairShader *hair = (HairShader *) self;
  int i = 0;

  LightSample *samples = NULL;
  const int nsamples = SlGetLightSampleCount(in);

  // allocate samples
  samples = SlNewLightSamples(in);

  out->Cs = Color();

  for (i = 0; i < nsamples; i++) {
    LightOutput Lout = {};
    Vector tangent;
    float diff = 0;
    float spec = 0;

    SlIlluminance(cxt, &samples[i], &in->P, &in->N, PI, in, &Lout);

    tangent = in->dPdv;
    Normalize(&tangent);

    diff = kajiya_diffuse(&tangent, &Lout.Ln);
    spec = kajiya_specular(&tangent, &Lout.Ln, &in->I);

    out->Cs.r += (in->Cd.r * hair->diffuse.r * diff + spec) * Lout.Cl.r;
    out->Cs.g += (in->Cd.g * hair->diffuse.g * diff + spec) * Lout.Cl.g;
    out->Cs.b += (in->Cd.b * hair->diffuse.b * diff + spec) * Lout.Cl.b;
  }

  SlFreeLightSamples(samples);

  out->Os = 1;
}

static int set_diffuse(void *self, const PropertyValue *value)
{
  HairShader *hair = (HairShader *) self;
  Color diffuse;

  diffuse.r = Max(0, value->vector[0]);
  diffuse.g = Max(0, value->vector[1]);
  diffuse.b = Max(0, value->vector[2]);
  hair->diffuse = diffuse;

  return 0;
}

static int set_specular(void *self, const PropertyValue *value)
{
  HairShader *hair = (HairShader *) self;
  Color specular;

  specular.r = Max(0, value->vector[0]);
  specular.g = Max(0, value->vector[1]);
  specular.b = Max(0, value->vector[2]);
  hair->specular = specular;

  return 0;
}

static int set_ambient(void *self, const PropertyValue *value)
{
  HairShader *hair = (HairShader *) self;
  Color ambient;

  ambient.r = Max(0, value->vector[0]);
  ambient.g = Max(0, value->vector[1]);
  ambient.b = Max(0, value->vector[2]);
  hair->ambient = ambient;

  return 0;
}

static int set_roughness(void *self, const PropertyValue *value)
{
  HairShader *hair = (HairShader *) self;
  float roughness = value->vector[0];

  roughness = Max(0, roughness);
  hair->roughness = roughness;

  return 0;
}

static int set_reflect(void *self, const PropertyValue *value)
{
  HairShader *hair = (HairShader *) self;
  Color reflect;

  reflect.r = Max(0, value->vector[0]);
  reflect.g = Max(0, value->vector[1]);
  reflect.b = Max(0, value->vector[2]);
  hair->reflect = reflect;

  return 0;
}

static float kajiya_diffuse(const Vector *tangent, const Vector *Ln)
{
  const float TL = Dot(*tangent, *Ln);
  const float diff = sqrt(1-TL*TL);

  return diff;
}

static float kajiya_specular(const Vector *tangent,
    const Vector *Ln, const Vector *I)
{
  float spec = 0;
  const float roughness = .05;
  const float TL = Dot(*tangent, *Ln);
  const float TI = Dot(*tangent, *I);

  assert(-1 <= TL && TL <= 1);
  assert(-1 <= TI && TI <= 1);

  spec = sqrt(1-TL*TL) * sqrt(1-TI*TI) + TL*TI;
  spec = pow(spec, 1/roughness);

  return spec;
}
