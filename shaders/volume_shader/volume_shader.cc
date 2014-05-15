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

using namespace fj;

struct VolumeShader {
  struct Color diffuse;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
    const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "VolumeShader";
static const struct ShaderFunctionTable MyFunctionTable = {
  MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
  {PROP_VECTOR3, "diffuse", {1, 1, 1, 0}, set_diffuse},
  {PROP_NONE,    NULL,      {0, 0, 0, 0}, NULL}
};

static const struct MetaInfo MyMetainfo[] = {
  {"help", "A volume shader."},
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
  struct VolumeShader *volume = FJ_MEM_ALLOC(struct VolumeShader);

  if (volume == NULL)
    return NULL;

  PropSetAllDefaultValues(volume, MyProperties);

  return volume;
}

static void MyFree(void *self)
{
  struct VolumeShader *volume = (struct VolumeShader *) self;
  if (volume == NULL)
    return;
  FJ_MEM_FREE(volume);
}

static void MyEvaluate(const void *self, const struct TraceContext *cxt,
    const struct SurfaceInput *in, struct SurfaceOutput *out)
{
  const struct VolumeShader *volume = (struct VolumeShader *) self;
  struct Color diff = {0, 0, 0};

  struct LightSample *samples = NULL;
  const int nsamples = SlGetLightSampleCount(in);

  int i = 0;

  /* allocate samples */
  samples = SlNewLightSamples(in);

  for (i = 0; i < nsamples; i++) {
    struct LightOutput Lout;

    SlIlluminance(cxt, &samples[i], &in->P, &in->N, N_PI, in, &Lout);

    /* diff */
    diff.r += Lout.Cl.r;
    diff.g += Lout.Cl.g;
    diff.b += Lout.Cl.b;
  }

  /* FJ_MEM_FREE samples */
  SlFreeLightSamples(samples);

  /* Cs */
  out->Cs.r = diff.r * volume->diffuse.r;
  out->Cs.g = diff.g * volume->diffuse.g;
  out->Cs.b = diff.b * volume->diffuse.b;

  /* Os */
  out->Os = 1;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
  struct VolumeShader *volume = (struct VolumeShader *) self;
  struct Color diffuse = {0, 0, 0};

  diffuse.r = Max(0, value->vector[0]);
  diffuse.g = Max(0, value->vector[1]);
  diffuse.b = Max(0, value->vector[2]);
  volume->diffuse = diffuse;

  return 0;
}

