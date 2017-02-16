// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"

using namespace fj;

class VolumeShader : public Shader {
public:
  VolumeShader() {}
  virtual ~VolumeShader() {}

public:
  Color diffuse;

private:
  virtual void evaluate(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;
  const Property *get_property_list() const;
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "VolumeShader";

static int set_diffuse(void *self, const PropertyValue *value);

static const Property MyPropertyList[] = {
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
      MyCreateFunction,
      MyDeleteFunction,
      MyPropertyList,
      MyMetainfo);
}
} // extern "C"

static void *MyCreateFunction(void)
{
  VolumeShader *volume = new VolumeShader();

  PropSetAllDefaultValues(volume, MyPropertyList);

  return volume;
}

static void MyDeleteFunction(void *self)
{
  VolumeShader *volume = (VolumeShader *) self;
  if (volume == NULL)
    return;
  delete volume;
}

void VolumeShader::evaluate(const TraceContext &cxt,
    const SurfaceInput &in, SurfaceOutput *out) const
{
  Color diff;

  LightSample *samples = NULL;
  const int nsamples = SlGetLightSampleCount(&in);

  // allocate samples
  samples = SlNewLightSamples(&in);

  for (int i = 0; i < nsamples; i++) {
    LightOutput Lout;

    SlIlluminance(&cxt, &samples[i], &in.P, &in.N, PI, &in, &Lout);

    // diff
    diff += Lout.Cl;
  }

  SlFreeLightSamples(samples);

  // Cs
  out->Cs = diff * diffuse;

  // Os
  out->Os = 1.0;
}

const Property *VolumeShader::get_property_list() const
{
  return MyPropertyList;
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
