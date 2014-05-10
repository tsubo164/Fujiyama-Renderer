/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_RAY_H
#define FJ_RAY_H

#include "fj_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POINT_ON_RAY(dst,orig,dir,t) do { \
  (dst)->x = (orig)->x + (t) * (dir)->x; \
  (dst)->y = (orig)->y + (t) * (dir)->y; \
  (dst)->z = (orig)->z + (t) * (dir)->z; \
  } while (0)

struct Ray {
  Ray() : orig(), dir(0, 0, 1), tmin(.001), tmax(1000) {}
  struct Vector orig;
  struct Vector dir;

  double tmin;
  double tmax;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
