/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_BOX_H
#define FJ_BOX_H

#include "fj_vector.h"
#include "fj_types.h"

namespace fj {

struct Box {
  Box() : min(), max() {}
  Box(Real xmin, Real ymin, Real zmin, Real xmax, Real ymax, Real zmax)
    : min(xmin, ymin, zmin), max(xmax, ymax, zmax) {}
  ~Box() {}

  Vector min;
  Vector max;
};

// edit
extern void BoxExpand(Box *box, Real delta);
extern void BoxReverseInfinite(Box *box);

// test
extern bool BoxContainsPoint(const Box &box, const Vector &point);
extern void BoxAddPoint(Box *box, const Vector &point);
extern void BoxAddBox(Box *box, const Box &otherbox);

extern bool BoxRayIntersect(const Box &box,
    const Vector &rayorig, const Vector &raydir,
    Real ray_tmin, Real ray_tmax,
    Real *hit_tmin, Real *hit_tmax);

// property
extern Vector BoxSize(const Box &box);
extern Vector BoxCentroid(const Box &box);
extern Real BoxDiagonal(const Box &box);

// print
extern void BoxPrint(const Box &box);

} // namespace xxx

#endif /* FJ_XXX_H */
