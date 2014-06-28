// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_VOLUME_FILLING_H
#define FJ_VOLUME_FILLING_H

#include "fj_compatibility.h"
#include "fj_vector.h"

namespace fj {

class Volume;

struct FJ_API CloudControlPoint {
  Vector orig;
  Vector udir;
  Vector vdir;
  Vector wdir;
  Vector noise_space;

  double density;
  double radius;
  double noise_amplitude;
};

struct FJ_API WispsControlPoint {
  Vector orig;
  Vector udir;
  Vector vdir;
  Vector wdir;
  Vector noise_space;

  double density;
  double radius;
  double noise_amplitude;

  double speck_count;
  double speck_radius;
};

FJ_API void LerpWispConstrolPoint(WispsControlPoint *cp,
    const WispsControlPoint *cp0, const WispsControlPoint *cp1,
    double t);

FJ_API void BilerpWispConstrolPoint(WispsControlPoint *cp,
    const WispsControlPoint *cp00, const WispsControlPoint *cp10,
    const WispsControlPoint *cp01, const WispsControlPoint *cp11,
    double s, double t);

FJ_API void FillWithSphere(Volume *volume,
    const Vector *center, double radius, float density);

} // namespace xxx

#endif // FJ_XXX_H
