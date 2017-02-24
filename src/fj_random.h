// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_RANDOM_H
#define FJ_RANDOM_H

#include "fj_compatibility.h"

namespace fj {

class Vector2;
class Vector;

class FJ_API XorShift {
public:
  XorShift();
  XorShift(unsigned int seed);
  ~XorShift() {}

  uint32_t NextInteger();
  double   NextFloat01();
  Vector   NextVector01();

  Vector SolidSphereRand();
  Vector HollowSphereRand();
  Vector SolidCubeRand();

  Vector2 SolidDiskRand();
  Vector2 HollowDiskRand();
  Vector2 GaussianDiskRand();

  double GaussianRand();
public:
  uint32_t state[4];
};

} // namespace xxx

#endif // FJ_XXX_H
