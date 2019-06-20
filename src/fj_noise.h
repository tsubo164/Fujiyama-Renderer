// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_NOISE_H
#define FJ_NOISE_H

#include "fj_compatibility.h"
#include "fj_types.h"

namespace  fj {

class Vector;

FJ_API Real PerlinNoise(const Vector &position,
    Real lacunarity, Real persistence, int octaves);

FJ_API Vector PerlinNoise3d(const Vector &position,
    Real lacunarity, Real persistence, int octaves);

FJ_API Real PeriodicNoise3d(Real x, Real y, Real z);

} // namespace xxx

#endif // FJ_XXX_H
