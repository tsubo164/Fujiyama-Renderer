// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_color.h"

#include <cstring>
#include <cstdio>

using namespace fj;

class ConstantShader : public Shader {
public:
  ConstantShader() {}
  virtual ~ConstantShader() {}

public:
  Color diffuse;
  Texture *texture;

private:
  virtual void evaluate(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;
  const Property *get_property_list() const;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out);

static const char MyPluginName[] = "ConstantShader";

static const ShaderFunctionTable MyFunctionTable = {
  MyEvaluate
};

static int set_diffuse(void *self, const PropertyValue *value);
static int set_texture(void *self, const PropertyValue *value);

static const Property MyProperties[] = {
  {PROP_VECTOR3, "diffuse", {1, 1, 1, 0}, set_diffuse},
  {PROP_TEXTURE, "texture", {0, 0, 0, 0}, set_texture},
  {PROP_NONE,    NULL,      {0, 0, 0, 0}, NULL}
};

static const MetaInfo MyMetainfo[] = {
  {"help", "A constant shader."},
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
  ConstantShader *constant = new ConstantShader();

  PropSetAllDefaultValues(constant, MyProperties);

  return constant;
}

static void MyFree(void *self)
{
  ConstantShader *constant = (ConstantShader *) self;
  if (constant == NULL)
    return;
  delete constant;
}

void ConstantShader::evaluate(const TraceContext &cxt,
    const SurfaceInput &in, SurfaceOutput *out) const
{
  Color4 C_tex;

  // C_tex
  if (texture != NULL) {
    C_tex = texture->Lookup(in.uv.u, in.uv.v);
    C_tex.r *= diffuse.r;
    C_tex.g *= diffuse.g;
    C_tex.b *= diffuse.b;
  } else {
    C_tex.r = diffuse.r;
    C_tex.g = diffuse.g;
    C_tex.b = diffuse.b;
  }

  // Cs
  out->Cs.r = C_tex.r;
  out->Cs.g = C_tex.g;
  out->Cs.b = C_tex.b;
  out->Os = 1;
}

const Property *ConstantShader::get_property_list() const
{
  return MyProperties;
}

static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out)
{
  const ConstantShader *constant = (ConstantShader *) self;
  Color4 C_tex;

  // C_tex
  if (constant->texture != NULL) {
    C_tex = constant->texture->Lookup(in->uv.u, in->uv.v);
    C_tex.r *= constant->diffuse.r;
    C_tex.g *= constant->diffuse.g;
    C_tex.b *= constant->diffuse.b;
  }
  else {
    C_tex.r = constant->diffuse.r;
    C_tex.g = constant->diffuse.g;
    C_tex.b = constant->diffuse.b;
  }

  // Cs
  out->Cs.r = C_tex.r;
  out->Cs.g = C_tex.g;
  out->Cs.b = C_tex.b;
  out->Os = 1;
}

static int set_diffuse(void *self, const PropertyValue *value)
{
  ConstantShader *constant = (ConstantShader *) self;
  Color diffuse;

  diffuse.r = Max(0, value->vector[0]);
  diffuse.g = Max(0, value->vector[1]);
  diffuse.b = Max(0, value->vector[2]);
  constant->diffuse = diffuse;

  return 0;
}

static int set_texture(void *self, const PropertyValue *value)
{
  ConstantShader *constant = (ConstantShader *) self;

  constant->texture = value->texture;

  return 0;
}
