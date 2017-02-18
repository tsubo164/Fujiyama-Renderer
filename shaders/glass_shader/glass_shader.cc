// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"

using namespace fj;

class GlassShader : public Shader {
public:
  GlassShader() {}
  virtual ~GlassShader() {}

public:
  Color diffuse;
  Color specular;
  Color ambient;

  Color filter_color;

  float roughness;
  float ior;

  int do_color_filter;

private:
  virtual void evaluate(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "GlassShader";

static int set_diffuse(void *self, const PropertyValue &value);
static int set_specular(void *self, const PropertyValue &value);
static int set_ambient(void *self, const PropertyValue &value);
static int set_filter_color(void *self, const PropertyValue &value);
static int set_roughness(void *self, const PropertyValue &value);
static int set_ior(void *self, const PropertyValue &value);

static const Property MyPropertyList[] = {
  Property("diffuse",      PropVector3(0, 0, 0), set_diffuse),
  Property("specular",     PropVector3(1, 1, 1), set_specular),
  Property("ambient",      PropVector3(1, 1, 1), set_ambient),
  Property("filter_color", PropVector3(1, 1, 1), set_filter_color),
  Property("roughness",    PropScalar(.1),       set_roughness),
  Property("ior",          PropScalar(1.4),      set_ior),
  Property()
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
      MyCreateFunction,
      MyDeleteFunction,
      MyPropertyList,
      MyMetainfo);
}
} // extern "C"

static void *MyCreateFunction(void)
{
  GlassShader *glass = new GlassShader();

  PropSetAllDefaultValues(glass, MyPropertyList);

  return glass;
}

static void MyDeleteFunction(void *self)
{
  GlassShader *glass = (GlassShader *) self;
  if (glass == NULL)
    return;
  delete glass;
}

void GlassShader::evaluate(const TraceContext &cxt,
    const SurfaceInput &in, SurfaceOutput *out) const
{
  TraceContext refl_cxt;
  TraceContext refr_cxt;
  Vector T;
  Vector R;
  Color4 C_refl;
  Color4 C_refr;
  double Kt = 0, Kr = 0;
  double t_hit = REAL_MAX;

  // Cs
  out->Cs = Color();

  Kr = SlFresnel(&in.I, &in.N, 1/ior);
  Kt = 1 - Kr;

  // reflect
  refl_cxt = SlReflectContext(&cxt, in.shaded_object);
  SlReflect(&in.I, &in.N, &R);
  Normalize(&R);
  // TODO fix hard-coded trace distance
  SlTrace(&refl_cxt, &in.P, &R, .0001, 1000, &C_refl, &t_hit);
  out->Cs.r += Kr * C_refl.r;
  out->Cs.g += Kr * C_refl.g;
  out->Cs.b += Kr * C_refl.b;

  // refract
  refr_cxt = SlRefractContext(&cxt, in.shaded_object);
  SlRefract(&in.I, &in.N, 1/ior, &T);
  Normalize(&T);
  SlTrace(&refr_cxt, &in.P, &T, .0001, 1000, &C_refr, &t_hit);

  if (do_color_filter && Dot(in.I, in.N) < 0) {
    C_refr.r *= pow(filter_color.r, t_hit);
    C_refr.g *= pow(filter_color.g, t_hit);
    C_refr.b *= pow(filter_color.b, t_hit);
  }

  out->Cs.r += Kt * C_refr.r;
  out->Cs.g += Kt * C_refr.g;
  out->Cs.b += Kt * C_refr.b;

  out->Os = 1;
}

static int set_diffuse(void *self, const PropertyValue &value)
{
  GlassShader *glass = (GlassShader *) self;
  Color diffuse;

  diffuse.r = Max(0, value.vector[0]);
  diffuse.g = Max(0, value.vector[1]);
  diffuse.b = Max(0, value.vector[2]);
  glass->diffuse = diffuse;

  return 0;
}

static int set_specular(void *self, const PropertyValue &value)
{
  GlassShader *glass = (GlassShader *) self;
  Color specular;

  specular.r = Max(0, value.vector[0]);
  specular.g = Max(0, value.vector[1]);
  specular.b = Max(0, value.vector[2]);
  glass->specular = specular;

  return 0;
}

static int set_ambient(void *self, const PropertyValue &value)
{
  GlassShader *glass = (GlassShader *) self;
  Color ambient;

  ambient.r = Max(0, value.vector[0]);
  ambient.g = Max(0, value.vector[1]);
  ambient.b = Max(0, value.vector[2]);
  glass->ambient = ambient;

  return 0;
}

static int set_filter_color(void *self, const PropertyValue &value)
{
  GlassShader *glass = (GlassShader *) self;
  Color filter_color;

  filter_color.r = Max(.001, value.vector[0]);
  filter_color.g = Max(.001, value.vector[1]);
  filter_color.b = Max(.001, value.vector[2]);
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

static int set_roughness(void *self, const PropertyValue &value)
{
  GlassShader *glass = (GlassShader *) self;
  float roughness = value.vector[0];

  roughness = Max(0, roughness);
  glass->roughness = roughness;

  return 0;
}

static int set_ior(void *self, const PropertyValue &value)
{
  GlassShader *glass = (GlassShader *) self;
  float ior = value.vector[0];

  ior = Max(0, ior);
  glass->ior = ior;

  return 0;
}
