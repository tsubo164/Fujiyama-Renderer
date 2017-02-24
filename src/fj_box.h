// Copyright (c) 2011-2017 Hiroshi Tsubokawa
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
  Box(const Vector &P0, const Vector &P1);
  ~Box() {}

  void Expand(Real delta);
  void ReverseInfinite();

  bool ContainsPoint(const Vector &point) const;
  void AddPoint(const Vector &point);
  void AddBox(const Box &other);

  Vector Centroid() const;
  Vector Diagonal() const;

public:
  Vector min;
  Vector max;
};

FJ_API bool BoxRayIntersect(const Box &box,
    const Vector &rayorig, const Vector &raydir,
    Real ray_tmin, Real ray_tmax,
    Real *hit_tmin, Real *hit_tmax);

FJ_API bool BoxBoxIntersect(const Box &a, const Box &b);

FJ_API std::ostream &operator<<(std::ostream &os, const Box &box);

} // namespace xxx

#endif // FJ_XXX_H
