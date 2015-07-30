// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_box.h"
#include "fj_numeric.h"

namespace fj {

void BoxExpand(Box *box, Real delta)
{
  box->min -= Vector(delta, delta, delta);
  box->max += Vector(delta, delta, delta);
}

void BoxReverseInfinite(Box *box)
{
  box->min = Vector( REAL_MAX,  REAL_MAX,  REAL_MAX);
  box->max = Vector(-REAL_MAX, -REAL_MAX, -REAL_MAX);
}

bool BoxContainsPoint(const Box &box, const Vector &point)
{
  if ((point[0] < box.min[0]) || (point[0] > box.max[0])) { return false; }
  if ((point[1] < box.min[1]) || (point[1] > box.max[1])) { return false; }
  if ((point[2] < box.min[2]) || (point[2] > box.max[2])) { return false; }

  return true;
}

void BoxAddPoint(Box *box, const Vector &point)
{
  box->min[0] = Min(box->min[0], point[0]);
  box->min[1] = Min(box->min[1], point[1]);
  box->min[2] = Min(box->min[2], point[2]);
  box->max[0] = Max(box->max[0], point[0]);
  box->max[1] = Max(box->max[1], point[1]);
  box->max[2] = Max(box->max[2], point[2]);
}

void BoxAddBox(Box *box, const Box &otherbox)
{
  box->min[0] = Min(box->min[0], otherbox.min[0]);
  box->min[1] = Min(box->min[1], otherbox.min[1]);
  box->min[2] = Min(box->min[2], otherbox.min[2]);
  box->max[0] = Max(box->max[0], otherbox.max[0]);
  box->max[1] = Max(box->max[1], otherbox.max[1]);
  box->max[2] = Max(box->max[2], otherbox.max[2]);
}

bool BoxRayIntersect(const Box &box,
    const Vector &rayorig, const Vector &raydir,
    Real ray_tmin, Real ray_tmax,
    Real *hit_tmin, Real *hit_tmax)
{
  Real tmin  = 0;
  Real tmax  = 0;
  Real tymin = 0;
  Real tymax = 0;
  Real tzmin = 0;
  Real tzmax = 0;

  if (raydir[0] >= 0) {
    tmin = (box.min[0] - rayorig[0]) / raydir[0];
    tmax = (box.max[0] - rayorig[0]) / raydir[0];
  } else {
    tmin = (box.max[0] - rayorig[0]) / raydir[0];
    tmax = (box.min[0] - rayorig[0]) / raydir[0];
  }

  if (raydir[1] >= 0) {
    tymin = (box.min[1] - rayorig[1]) / raydir[1];
    tymax = (box.max[1] - rayorig[1]) / raydir[1];
  } else {
    tymin = (box.max[1] - rayorig[1]) / raydir[1];
    tymax = (box.min[1] - rayorig[1]) / raydir[1];
  }

  if ((tmin > tymax) || (tymin > tmax)) {
    return 0;
  }

  if (tymin > tmin) {
    tmin = tymin;
  }
  if (tymax < tmax) {
    tmax = tymax;
  }

  if (raydir[2] >= 0) {
    tzmin = (box.min[2] - rayorig[2]) / raydir[2];
    tzmax = (box.max[2] - rayorig[2]) / raydir[2];
  } else {
    tzmin = (box.max[2] - rayorig[2]) / raydir[2];
    tzmax = (box.min[2] - rayorig[2]) / raydir[2];
  }

  if ((tmin > tzmax) || (tzmin > tmax)) {
    return 0;
  }

  if (tzmin > tmin) {
    tmin = tzmin;
  }
  if (tzmax < tmax) {
    tmax = tzmax;
  }

  const bool hit = ((tmin < ray_tmax) && (tmax > ray_tmin));
  if (hit) {
    *hit_tmin = tmin;
    *hit_tmax = tmax;
  }

  return hit;
}

bool BoxBoxIntersect(const Box &a, const Box &b)
{
  if (a.max[0] < b.min[0] ||
      a.min[0] > b.max[0] ||
      a.max[1] < b.min[1] ||
      a.min[1] > b.max[1] ||
      a.max[2] < b.min[2] ||
      a.min[2] > b.max[2]) {
    return false;
  } else {
    return true;
  }
}

Vector BoxCentroid(const Box &box)
{
  return .5 * (box.min + box.max);
}

Vector BoxDiagonal(const Box &box)
{
  return box.max - box.min;
}

std::ostream &operator<<(std::ostream &os, const Box &box)
{
  return os << "(" << box.min << ", " << box.max << ")";
}

} // namespace xxx
