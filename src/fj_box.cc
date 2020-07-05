// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_box.h"
#include "fj_numeric.h"

namespace fj {

Box::Box(const Vector &P0, const Vector &P1)
{
  for (int i = 0; i < 3; i++) {
    if (P0[i] < P1[i]) {
      min[i] = P0[i];
      max[i] = P1[i];
    } else {
      min[i] = P1[i];
      max[i] = P0[i];
    }
  }
}

void Box::Expand(Real delta)
{
  min -= Vector(delta, delta, delta);
  max += Vector(delta, delta, delta);
}

void Box::ReverseInfinite()
{
  min = Vector( REAL_MAX,  REAL_MAX,  REAL_MAX);
  max = Vector(-REAL_MAX, -REAL_MAX, -REAL_MAX);
}

bool Box::ContainsPoint(const Vector &point) const
{
  if ((point[0] < min[0]) || (max[0] < point[0])) { return false; }
  if ((point[1] < min[1]) || (max[1] < point[1])) { return false; }
  if ((point[2] < min[2]) || (max[2] < point[2])) { return false; }

  return true;
}

void Box::AddPoint(const Vector &point)
{
  min[0] = Min(min[0], point[0]);
  min[1] = Min(min[1], point[1]);
  min[2] = Min(min[2], point[2]);
  max[0] = Max(max[0], point[0]);
  max[1] = Max(max[1], point[1]);
  max[2] = Max(max[2], point[2]);
}

void Box::AddBox(const Box &other)
{
  min[0] = Min(min[0], other.min[0]);
  min[1] = Min(min[1], other.min[1]);
  min[2] = Min(min[2], other.min[2]);
  max[0] = Max(max[0], other.max[0]);
  max[1] = Max(max[1], other.max[1]);
  max[2] = Max(max[2], other.max[2]);
}

Vector Box::Centroid() const
{
  return .5 * (min + max);
}

Vector Box::Diagonal() const
{
  return max - min;
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

std::ostream &operator<<(std::ostream &os, const Box &box)
{
  return os << "(" << box.min << ", " << box.max << ")";
}

} // namespace xxx
