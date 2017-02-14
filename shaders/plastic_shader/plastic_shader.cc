// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_color.h"

#include <cstring>
#include <cstdio>
#include <cfloat>

using namespace fj;

class PlasticShader : public Shader {
public:
  PlasticShader() {}
  virtual ~PlasticShader() {}

public:
  Color diffuse;
  Color specular;
  Color ambient;
  float roughness;

  Color reflect;
  float ior;

  float opacity;

  int do_reflect;

  Texture *diffuse_map;
  Texture *bump_map;
  float bump_amplitude;

private:
  virtual void evaluate(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;
  const Property *get_property_list() const;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out);

static const char MyPluginName[] = "PlasticShader";
static const ShaderFunctionTable MyFunctionTable = {
  MyEvaluate
};

static int set_diffuse(void *self, const PropertyValue *value);
static int set_specular(void *self, const PropertyValue *value);
static int set_ambient(void *self, const PropertyValue *value);
static int set_roughness(void *self, const PropertyValue *value);
static int set_reflect(void *self, const PropertyValue *value);
static int set_ior(void *self, const PropertyValue *value);
static int set_opacity(void *self, const PropertyValue *value);
static int set_diffuse_map(void *self, const PropertyValue *value);
static int set_bump_map(void *self, const PropertyValue *value);
static int set_bump_amplitude(void *self, const PropertyValue *value);

static const Property MyProperties[] = {
  {PROP_VECTOR3, "diffuse",        {.8, .8, .8, 0}, set_diffuse},
  {PROP_VECTOR3, "specular",       {1, 1, 1, 0},    set_specular},
  {PROP_VECTOR3, "ambient",        {1, 1, 1, 0},    set_ambient},
  {PROP_SCALAR,  "roughness",      {.1, 0, 0, 0},   set_roughness},
  {PROP_VECTOR3, "reflect",        {1, 1, 1, 0},    set_reflect},
  {PROP_SCALAR,  "ior",            {1.4, 0, 0, 0},  set_ior},
  {PROP_SCALAR,  "opacity",        {1, 0, 0, 0},    set_opacity},
  {PROP_TEXTURE, "diffuse_map",    {0, 0, 0, 0},    set_diffuse_map},
  {PROP_TEXTURE, "bump_map",       {0, 0, 0, 0},    set_bump_map},
  {PROP_SCALAR,  "bump_amplitude", {1, 0, 0, 0},    set_bump_amplitude},
  {PROP_NONE,    NULL,             {0, 0, 0, 0},    NULL}
};

static const MetaInfo MyMetainfo[] = {
  {"help", "A plastic shader."},
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
  PlasticShader *plastic = new PlasticShader();
  PropSetAllDefaultValues(plastic, MyProperties);

  return plastic;
}

static void MyFree(void *self)
{
  PlasticShader *plastic = (PlasticShader *) self;
  if (plastic == NULL) {
    return;
  }
  delete plastic;
}

void PlasticShader::evaluate(const TraceContext &cxt,
    const SurfaceInput &in, SurfaceOutput *out) const
{
  Color diff;
  Color spec;
  Color4 diff_map(1, 1, 1, 1);
  Vector Nf;
  int i = 0;

  LightSample *samples = NULL;
  const int nsamples = SlGetLightSampleCount(&in);

  SlFaceforward(&in.I, &in.N, &Nf);

  // bump map
  if (bump_map != NULL) {
    Vector N_bump;
    SlBumpMapping(bump_map,
        &in.dPdu, &in.dPdv,
        &in.uv, bump_amplitude,
        &Nf, &N_bump);
    Nf = N_bump;
  }

  // allocate samples
  samples = SlNewLightSamples(&in);

  for (i = 0; i < nsamples; i++) {
    LightOutput Lout;
    float Kd = 0;
    SlIlluminance(&cxt, &samples[i], &in.P, &Nf, PI / 2., &in, &Lout);
    // spec
#if 0
    Ks = SlPhong(in.I, Nf, Ln, .05);
#endif

    // diff
    Kd = Dot(Nf, Lout.Ln);
    Kd = Max(0, Kd);
    diff.r += Kd * Lout.Cl.r;
    diff.g += Kd * Lout.Cl.g;
    diff.b += Kd * Lout.Cl.b;
  }

  // free samples
  SlFreeLightSamples(samples);

  // diffuse map
  if (diffuse_map != NULL) {
    diff_map = diffuse_map->Lookup(in.uv.u, in.uv.v);
  }

  // Cs
  out->Cs.r = diff.r * diffuse.r * diff_map.r + spec.r;
  out->Cs.g = diff.g * diffuse.g * diff_map.g + spec.g;
  out->Cs.b = diff.b * diffuse.b * diff_map.b + spec.b;

  // reflect
  if (do_reflect) {
    Color4 C_refl;
    Vector R;
    double t_hit = FLT_MAX;
    double Kr = 0;

    const TraceContext refl_cxt = SlReflectContext(&cxt, in.shaded_object);

    SlReflect(&in.I, &Nf, &R);
    Normalize(&R);
    SlTrace(&refl_cxt, &in.P, &R, .001, 1000, &C_refl, &t_hit);

    Kr = SlFresnel(&in.I, &Nf, 1/ior);
    out->Cs.r += Kr * C_refl.r * reflect.r;
    out->Cs.g += Kr * C_refl.g * reflect.g;
    out->Cs.b += Kr * C_refl.b * reflect.b;
  }

  out->Os = 1;
  out->Os = opacity;
}

const Property *PlasticShader::get_property_list() const
{
  return MyProperties;
}

static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out)
{
  const PlasticShader *plastic = (PlasticShader *) self;
  Color diff;
  Color spec;
  Color4 diff_map(1, 1, 1, 1);
  Vector Nf;
  int i = 0;

  LightSample *samples = NULL;
  const int nsamples = SlGetLightSampleCount(in);

  SlFaceforward(&in->I, &in->N, &Nf);

  // bump map
  if (plastic->bump_map != NULL) {
    Vector N_bump;
    SlBumpMapping(plastic->bump_map,
        &in->dPdu, &in->dPdv,
        &in->uv, plastic->bump_amplitude,
        &Nf, &N_bump);
    Nf = N_bump;
  }

  // allocate samples
  samples = SlNewLightSamples(in);

  for (i = 0; i < nsamples; i++) {
    LightOutput Lout;
    float Kd = 0;
    SlIlluminance(cxt, &samples[i], &in->P, &Nf, PI / 2., in, &Lout);
    // spec
#if 0
    Ks = SlPhong(in->I, Nf, Ln, .05);
#endif

    // diff
    Kd = Dot(Nf, Lout.Ln);
    Kd = Max(0, Kd);
    diff.r += Kd * Lout.Cl.r;
    diff.g += Kd * Lout.Cl.g;
    diff.b += Kd * Lout.Cl.b;
  }

  // free samples
  SlFreeLightSamples(samples);

  // diffuse map
  if (plastic->diffuse_map != NULL) {
    diff_map = plastic->diffuse_map->Lookup(in->uv.u, in->uv.v);
  }

  // Cs
  out->Cs.r = diff.r * plastic->diffuse.r * diff_map.r + spec.r;
  out->Cs.g = diff.g * plastic->diffuse.g * diff_map.g + spec.g;
  out->Cs.b = diff.b * plastic->diffuse.b * diff_map.b + spec.b;

  // reflect
  if (plastic->do_reflect) {
    Color4 C_refl;
    Vector R;
    double t_hit = FLT_MAX;
    double Kr = 0;

    const TraceContext refl_cxt = SlReflectContext(cxt, in->shaded_object);

    SlReflect(&in->I, &Nf, &R);
    Normalize(&R);
    SlTrace(&refl_cxt, &in->P, &R, .001, 1000, &C_refl, &t_hit);

    Kr = SlFresnel(&in->I, &Nf, 1/plastic->ior);
    out->Cs.r += Kr * C_refl.r * plastic->reflect.r;
    out->Cs.g += Kr * C_refl.g * plastic->reflect.g;
    out->Cs.b += Kr * C_refl.b * plastic->reflect.b;
  }

  out->Os = 1;
  out->Os = plastic->opacity;
}

static int set_diffuse(void *self, const PropertyValue *value)
{
  PlasticShader *plastic = (PlasticShader *) self;
  Color diffuse;

  diffuse.r = Max(0, value->vector[0]);
  diffuse.g = Max(0, value->vector[1]);
  diffuse.b = Max(0, value->vector[2]);
  plastic->diffuse = diffuse;

  return 0;
}

static int set_specular(void *self, const PropertyValue *value)
{
  PlasticShader *plastic = (PlasticShader *) self;
  Color specular;

  specular.r = Max(0, value->vector[0]);
  specular.g = Max(0, value->vector[1]);
  specular.b = Max(0, value->vector[2]);
  plastic->specular = specular;

  return 0;
}

static int set_ambient(void *self, const PropertyValue *value)
{
  PlasticShader *plastic = (PlasticShader *) self;
  Color ambient;

  ambient.r = Max(0, value->vector[0]);
  ambient.g = Max(0, value->vector[1]);
  ambient.b = Max(0, value->vector[2]);
  plastic->ambient = ambient;

  return 0;
}

static int set_roughness(void *self, const PropertyValue *value)
{
  PlasticShader *plastic = (PlasticShader *) self;
  float roughness = value->vector[0];

  roughness = Max(0, roughness);
  plastic->roughness = roughness;

  return 0;
}

static int set_reflect(void *self, const PropertyValue *value)
{
  PlasticShader *plastic = (PlasticShader *) self;
  Color reflect;

  reflect.r = Max(0, value->vector[0]);
  reflect.g = Max(0, value->vector[1]);
  reflect.b = Max(0, value->vector[2]);
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

static int set_ior(void *self, const PropertyValue *value)
{
  PlasticShader *plastic = (PlasticShader *) self;
  float ior = value->vector[0];

  ior = Max(.001, ior);
  plastic->ior = ior;

  return 0;
}

static int set_opacity(void *self, const PropertyValue *value)
{
  PlasticShader *plastic = (PlasticShader *) self;
  float opacity = value->vector[0];

  opacity = Clamp(opacity, 0, 1);
  plastic->opacity = opacity;

  return 0;
}

static int set_diffuse_map(void *self, const PropertyValue *value)
{
  PlasticShader *plastic = (PlasticShader *) self;

  plastic->diffuse_map = value->texture;

  return 0;
}

static int set_bump_map(void *self, const PropertyValue *value)
{
  PlasticShader *plastic = (PlasticShader *) self;

  plastic->bump_map = value->texture;

  return 0;
}

static int set_bump_amplitude(void *self, const PropertyValue *value)
{
  PlasticShader *plastic = (PlasticShader *) self;
  const float amp = value->vector[0];

  plastic->bump_amplitude = amp;

  return 0;
}
