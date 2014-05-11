/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_SHADER_H
#define FJ_SHADER_H

#include "fj_property.h"
#include "fj_shading.h"
#include "fj_texture.h"
#include "fj_plugin.h"
#include "fj_light.h"
#include <stddef.h>

#define SHADER_PLUGIN_TYPE "Shader"

namespace fj {

struct Shader;

struct ShaderFunctionTable {
  void (*MyEvaluate)(const void *self, const struct TraceContext *cxt,
      const struct SurfaceInput *in, struct SurfaceOutput *out);
};

enum ShdErrorNo {
  SHD_ERR_NOERR = 0,
  SHD_ERR_TYPE_NOT_MATCH,
  SHD_ERR_NOOBJ,
  SHD_ERR_NOVTBL,
  SHD_ERR_NOMEM
};

extern struct Shader *ShdNew(const struct Plugin *plg);
extern void ShdFree(struct Shader *shader);

extern void ShdEvaluate(const struct Shader *shader, const struct TraceContext *cxt,
    const struct SurfaceInput *in, struct SurfaceOutput *out);

extern const struct Property *ShdGetPropertyList(const struct Shader *shader);
extern int ShdSetProperty(struct Shader *shader,
    const char *prop_name, const struct PropertyValue *src_data);

} // namespace fj

#endif /* FJ_XXX_H */
