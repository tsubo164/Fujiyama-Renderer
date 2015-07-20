// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_BOX_H
#define FJ_BOX_H

#include "fj_vector.h"
#include "fj_types.h"
#include <iostream>

namespace fj {

class FJ_API Box {
public:
  Box() : min(), max() {}
  Box(Real xmin, Real ymin, Real zmin, Real xmax, Real ymax, Real zmax)
    : min(xmin, ymin, zmin), max(xmax, ymax, zmax) {}
  ~Box() {}

public:
  Vector min;
  Vector max;
};

FJ_API void BoxExpand(Box *box, Real delta);
FJ_API void BoxReverseInfinite(Box *box);

FJ_API bool BoxContainsPoint(const Box &box, const Vector &point);
FJ_API void BoxAddPoint(Box *box, const Vector &point);
FJ_API void BoxAddBox(Box *box, const Box &otherbox);

FJ_API bool BoxRayIntersect(const Box &box,
    const Vector &rayorig, const Vector &raydir,
    Real ray_tmin, Real ray_tmax,
    Real *hit_tmin, Real *hit_tmax);

FJ_API Vector BoxCentroid(const Box &box);
FJ_API Vector BoxDiagonal(const Box &box);

FJ_API std::ostream &operator<<(std::ostream &os, const Box &box);

} // namespace xxx

#endif // FJ_XXX_H
