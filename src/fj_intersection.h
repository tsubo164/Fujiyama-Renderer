/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_INTERSECTION_H
#define FJ_INTERSECTION_H

#include "fj_tex_coord.h"
#include "fj_vector.h"
#include "fj_color.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ObjectInstance;

struct Intersection {
  struct Vector P;
  struct Vector N;
  struct Color Cd;
  struct TexCoord uv;

  struct Vector dPdu;
  struct Vector dPdv;

  const struct ObjectInstance *object;
  int prim_id;

  double t_hit;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */

