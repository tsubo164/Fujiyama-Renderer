// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"

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
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "ConstantShader";

static int set_diffuse(void *self, const PropertyValue &value);
static int set_texture(void *self, const PropertyValue &value);

static const Property MyPropertyList[] = {
  Property("diffuse", PropVector3(1, 1, 1), set_diffuse),
  Property("texture", PropTexture(NULL),    set_texture),
  Property()
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
      MyCreateFunction,
      MyDeleteFunction,
      MyPropertyList,
      MyMetainfo);
}
} // extern "C"

static void *MyCreateFunction(void)
{
  ConstantShader *constant = new ConstantShader();

  PropSetAllDefaultValues(constant, MyPropertyList);

  return constant;
}

static void MyDeleteFunction(void *self)
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

static int set_diffuse(void *self, const PropertyValue &value)
{
  ConstantShader *constant = (ConstantShader *) self;
  Color diffuse;

  diffuse.r = Max(0, value.vector[0]);
  diffuse.g = Max(0, value.vector[1]);
  diffuse.b = Max(0, value.vector[2]);
  constant->diffuse = diffuse;

  return 0;
}

static int set_texture(void *self, const PropertyValue &value)
{
  ConstantShader *constant = (ConstantShader *) self;

  constant->texture = value.texture;

  return 0;
}
