// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_INTERSECTION_H
#define FJ_INTERSECTION_H

#include "fj_compatibility.h"
#include "fj_object_instance.h"
#include "fj_tex_coord.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_types.h"

#include <cstddef>

namespace fj {

class Shader;

class Intersection {
public:
  Intersection() :
      object(NULL),
      prim_id(0),
      shading_group_id(0),
      t_hit(REAL_MAX) {}
  ~Intersection() {}

  Vector P;
  Vector N;
  Color Cd;
  TexCoord uv;

  Vector dPdu;
  Vector dPdv;

  const ObjectInstance *object;
  int prim_id;
  int shading_group_id;

  Real t_hit;

  const Shader *GetShader() const
  {
    return object->GetShader(shading_group_id);
  }
};

} // namespace xxx

#endif // FJ_XXX_H
