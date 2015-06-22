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

class VolumeShader {
public:
  VolumeShader() {}
  ~VolumeShader() {}

public:
  Color diffuse;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out);

static const char MyPluginName[] = "VolumeShader";
static const ShaderFunctionTable MyFunctionTable = {
  MyEvaluate
};

static int set_diffuse(void *self, const PropertyValue *value);

static const Property MyProperties[] = {
  {PROP_VECTOR3, "diffuse", {1, 1, 1, 0}, set_diffuse},
  {PROP_NONE,    NULL,      {0, 0, 0, 0}, NULL}
};

static const MetaInfo MyMetainfo[] = {
  {"help", "A volume shader."},
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
  VolumeShader *volume = new VolumeShader();

  PropSetAllDefaultValues(volume, MyProperties);

  return volume;
}

static void MyFree(void *self)
{
  VolumeShader *volume = (VolumeShader *) self;
  if (volume == NULL)
    return;
  delete volume;
}

static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out)
{
  const VolumeShader *volume = (VolumeShader *) self;
  Color diff;

  LightSample *samples = NULL;
  const int nsamples = SlGetLightSampleCount(in);

  int i = 0;

  // allocate samples
  samples = SlNewLightSamples(in);

  for (i = 0; i < nsamples; i++) {
    LightOutput Lout;

    SlIlluminance(cxt, &samples[i], &in->P, &in->N, PI, in, &Lout);

    // diff
    diff.r += Lout.Cl.r;
    diff.g += Lout.Cl.g;
    diff.b += Lout.Cl.b;
  }

  SlFreeLightSamples(samples);

  // Cs
  out->Cs.r = diff.r * volume->diffuse.r;
  out->Cs.g = diff.g * volume->diffuse.g;
  out->Cs.b = diff.b * volume->diffuse.b;

  // Os
  out->Os = 1;
}

static int set_diffuse(void *self, const PropertyValue *value)
{
  VolumeShader *volume = (VolumeShader *) self;
  Color diffuse;

  diffuse.r = Max(0, value->vector[0]);
  diffuse.g = Max(0, value->vector[1]);
  diffuse.b = Max(0, value->vector[2]);
  volume->diffuse = diffuse;

  return 0;
}
