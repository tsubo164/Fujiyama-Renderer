// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SHADER_H
#define FJ_SHADER_H

#include "fj_compatibility.h"
#include "fj_numeric.h"
#include "fj_shading.h"
#include "fj_texture.h"
#include "fj_plugin.h"
#include "fj_random.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_light.h"
#include <string>

// TODO const char *GetPluginType();
#define SHADER_PLUGIN_TYPE "Shader"

namespace fj {

class FJ_API Shader {
public:
  Shader();
  virtual ~Shader();

  void Evaluate(const TraceContext &cxt, const SurfaceInput &in, SurfaceOutput *out) const;

private:
  virtual void evaluate(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const = 0;
};

} // namespace xxx

#endif // FJ_XXX_H
