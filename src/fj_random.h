// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_RANDOM_H
#define FJ_RANDOM_H

#include "fj_compatibility.h"

namespace fj {

class Vector2;
class Vector;

class FJ_API XorShift {
public:
  XorShift(unsigned int seed);

  XorShift() :state() {}
  ~XorShift() {}

public:
  uint32_t state[4];
};

// TODO MOVE TO CONSTRUCTOR
FJ_API void XorInit(XorShift *xr);

FJ_API uint32_t XorNextInteger(XorShift *xr);
FJ_API double XorNextFloat01(XorShift *xr);

FJ_API void XorSolidSphereRand(XorShift *xr, Vector *out_position);
FJ_API void XorHollowSphereRand(XorShift *xr, Vector *out_position);
FJ_API void XorSolidCubeRand(XorShift *xr, Vector *out_position);

FJ_API void XorSolidDiskRand(XorShift *xr, Vector2 *out_position);
FJ_API void XorHollowDiskRand(XorShift *xr, Vector2 *out_position);
FJ_API void XorGaussianDiskRand(XorShift *xr, Vector2 *out_position);

FJ_API double XorGaussianRand(XorShift *xr);

} // namespace xxx

#endif // FJ_XXX_H
