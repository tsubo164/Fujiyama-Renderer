// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_PIXEL_SAMPLER_H
#define FJ_PIXEL_SAMPLER_H

#include "fj_vector.h"
#include "fj_color.h"
#include "fj_types.h"

namespace fj {

class Sample {
public:
  Sample() : uv(), data(), time(0.) {}
  ~Sample() {}

public:
  Vector2 uv;
  Vector4 data;
  Real time;
};

inline Vector4 ToData(const Color4 &color)
{
  return Vector4(color[0], color[1], color[2], color[3]);
}

} // namespace xxx

#endif // FJ_XXX_H
