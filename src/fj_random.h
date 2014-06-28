// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_RANDOM_H
#define FJ_RANDOM_H

#include "fj_compatibility.h"

namespace fj {

struct Vector2;
struct Vector;

struct FJ_API XorShift {
  uint32_t state[4];
};

FJ_API void XorInit(struct XorShift *xr);

FJ_API uint32_t XorNextInteger(struct XorShift *xr);
FJ_API double XorNextFloat01(struct XorShift *xr);

FJ_API void XorSolidSphereRand(struct XorShift *xr, struct Vector *out_position);
FJ_API void XorHollowSphereRand(struct XorShift *xr, struct Vector *out_position);
FJ_API void XorSolidCubeRand(struct XorShift *xr, struct Vector *out_position);

FJ_API void XorSolidDiskRand(struct XorShift *xr, struct Vector2 *out_position);
FJ_API void XorHollowDiskRand(struct XorShift *xr, struct Vector2 *out_position);
FJ_API void XorGaussianDiskRand(struct XorShift *xr, struct Vector2 *out_position);

FJ_API double XorGaussianRand(struct XorShift *xr);

} // namespace xxx

#endif // FJ_XXX_H
