// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SHADER_H
#define FJ_SHADER_H

#include "fj_property.h"
#include "fj_shading.h"
#include "fj_texture.h"
#include "fj_plugin.h"
#include "fj_light.h"
#include <string>

#define SHADER_PLUGIN_TYPE "Shader"

namespace fj {

class ShaderFunctionTable {
public:
  void (*MyEvaluate)(const void *self, const TraceContext *cxt,
      const SurfaceInput *in, SurfaceOutput *out);
};

enum ShdErrorNo {
  SHD_ERR_NOERR = 0,
  SHD_ERR_TYPE_NOT_MATCH,
  SHD_ERR_NOOBJ,
  SHD_ERR_NOVTBL,
  SHD_ERR_NOMEM
};

class Shader {
public:
  Shader();
  ~Shader();

  int Initialize(const Plugin *plugin);
  void Evaluate(const TraceContext &cxt, const SurfaceInput &in, SurfaceOutput *out) const;

  const Property *GetPropertyList() const;
  int SetProperty(const std::string &prop_name, const PropertyValue &src_data) const;

public:
  void *self_;
  const ShaderFunctionTable *vptr_;
  const Plugin *plugin_;
};

extern Shader *ShdNew(const Plugin *plg);
extern void ShdFree(Shader *shader);

} // namespace xxx

#endif // FJ_XXX_H
