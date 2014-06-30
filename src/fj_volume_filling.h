// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_VOLUME_FILLING_H
#define FJ_VOLUME_FILLING_H

#include "fj_compatibility.h"
#include "fj_vector.h"
#include "fj_types.h"

namespace fj {

class Volume;

class FJ_API CloudControlPoint {
public:
  CloudControlPoint() {}
  ~CloudControlPoint() {}

public:
  Vector orig;
  Vector udir;
  Vector vdir;
  Vector wdir;
  Vector noise_space;

  Real density;
  Real radius;
  Real noise_amplitude;
};

class FJ_API WispsControlPoint {
public:
  WispsControlPoint() {}
  ~WispsControlPoint() {}

public:
  Vector orig;
  Vector udir;
  Vector vdir;
  Vector wdir;
  Vector noise_space;

  Real density;
  Real radius;
  Real noise_amplitude;

  Real speck_count;
  Real speck_radius;
};

FJ_API void LerpWispConstrolPoint(WispsControlPoint *cp,
    const WispsControlPoint *cp0, const WispsControlPoint *cp1,
    Real t);

FJ_API void BilerpWispConstrolPoint(WispsControlPoint *cp,
    const WispsControlPoint *cp00, const WispsControlPoint *cp10,
    const WispsControlPoint *cp01, const WispsControlPoint *cp11,
    Real s, Real t);

FJ_API void FillWithSphere(Volume *volume,
    const Vector *center, Real radius, float density);

} // namespace xxx

#endif // FJ_XXX_H
