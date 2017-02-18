// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_PROCEDURE_H
#define FJ_PROCEDURE_H

#include "fj_compatibility.h"
#include "fj_volume_filling.h"
#include "fj_turbulence.h"
#include "fj_progress.h"
#include "fj_numeric.h"
#include "fj_shading.h"
#include "fj_texture.h"
#include "fj_plugin.h"
#include "fj_random.h"
#include "fj_vector.h"
#include "fj_volume.h"
#include "fj_color.h"
#include "fj_light.h"
#include <string>

// TODO const char *GetPluginType();
#define PROCEDURE_PLUGIN_TYPE "Procedure"

namespace fj {

class FJ_API Procedure {
public:
  Procedure();
  virtual ~Procedure();

  int Run() const;

private:
  virtual int run() const = 0;
};

} // namespace xxx

#endif // FJ_XXX_H
