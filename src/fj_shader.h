// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SHADER_H
#define FJ_SHADER_H

#include "fj_compatibility.h"
#include "fj_property.h"
#include "fj_shading.h"
#include "fj_texture.h"
#include "fj_plugin.h"
#include "fj_light.h"
#include <string>

#define SHADER_PLUGIN_TYPE "Shader"

namespace fj {

class FJ_API ShaderFunctionTable {
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

class FJ_API Shader {
public:
  Shader();
  virtual ~Shader();

  void Evaluate(const TraceContext &cxt, const SurfaceInput &in, SurfaceOutput *out) const;
  int SetProperty(const std::string &prop_name, const PropertyValue &src_data);

private:
  virtual void evaluate(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const = 0;
  virtual const Property *get_property_list() const = 0;
};

} // namespace xxx

#endif // FJ_XXX_H
