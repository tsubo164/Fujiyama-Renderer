/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_box.h"
#include "fj_numeric.h"

#include <cstdio>
#include <cfloat>

namespace fj {

void BoxExpand(Box *box, Real delta)
{
  box->min -= Vector(delta, delta, delta);
  box->max += Vector(delta, delta, delta);
}

void BoxReverseInfinite(Box *box)
{
  box->min = Vector( FLT_MAX,  FLT_MAX,  FLT_MAX);
  box->max = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

bool BoxContainsPoint(const Box &box, const Vector &point)
{
  if ((point.x < box.min.x) || (point.x > box.max.x)) return false;
  if ((point.y < box.min.y) || (point.y > box.max.y)) return false;
  if ((point.z < box.min.z) || (point.z > box.max.z)) return false;

  return true;
}

void BoxAddPoint(Box *box, const Vector &point)
{
  box->min.x = Min(box->min.x, point.x);
  box->min.y = Min(box->min.y, point.y);
  box->min.z = Min(box->min.z, point.z);
  box->max.x = Max(box->max.x, point.x);
  box->max.y = Max(box->max.y, point.y);
  box->max.z = Max(box->max.z, point.z);
}

void BoxAddBox(Box *box, const Box &otherbox)
{
  box->min.x = Min(box->min.x, otherbox.min.x);
  box->min.y = Min(box->min.y, otherbox.min.y);
  box->min.z = Min(box->min.z, otherbox.min.z);
  box->max.x = Max(box->max.x, otherbox.max.x);
  box->max.y = Max(box->max.y, otherbox.max.y);
  box->max.z = Max(box->max.z, otherbox.max.z);
}

bool BoxRayIntersect(const Box &box,
    const Vector &rayorig, const Vector &raydir,
    Real ray_tmin, Real ray_tmax,
    Real *hit_tmin, Real *hit_tmax)
{
  Real tmin, tmax, tymin, tymax, tzmin, tzmax;

  if (raydir.x >= 0) {
    tmin = (box.min.x - rayorig.x) / raydir.x;
    tmax = (box.max.x - rayorig.x) / raydir.x;
  } else {
    tmin = (box.max.x - rayorig.x) / raydir.x;
    tmax = (box.min.x - rayorig.x) / raydir.x;
  }

  if (raydir.y >= 0) {
    tymin = (box.min.y - rayorig.y) / raydir.y;
    tymax = (box.max.y - rayorig.y) / raydir.y;
  } else {
    tymin = (box.max.y - rayorig.y) / raydir.y;
    tymax = (box.min.y - rayorig.y) / raydir.y;
  }

  if ((tmin > tymax) || (tymin > tmax))
    return 0;

  if (tymin > tmin)
    tmin = tymin;
  if (tymax < tmax)
    tmax = tymax;

  if (raydir.z >= 0) {
    tzmin = (box.min.z - rayorig.z) / raydir.z;
    tzmax = (box.max.z - rayorig.z) / raydir.z;
  } else {
    tzmin = (box.max.z - rayorig.z) / raydir.z;
    tzmax = (box.min.z - rayorig.z) / raydir.z;
  }

  if ((tmin > tzmax) || (tzmin > tmax))
    return 0;

  if (tzmin > tmin)
    tmin = tzmin;
  if (tzmax < tmax)
    tmax = tzmax;

  const bool hit = ((tmin < ray_tmax) && (tmax > ray_tmin));
  if (hit) {
    *hit_tmin = tmin;
    *hit_tmax = tmax;
  }

  return hit;
}

Vector BoxSize(const Box &box)
{
  return box.max - box.min;
}

Vector BoxCentroid(const Box &box)
{
  return .5 * (box.min + box.max);
}

Real BoxDiagonal(const Box &box)
{
  const Vector size = BoxSize(box);
  return .5 * Length(size);
}

void BoxPrint(const Box &box)
{
  printf("(%g, %g, %g) (%g, %g, %g)\n",
      box.min.x, box.min.y, box.min.z,
      box.max.x, box.max.y, box.max.z);
}

} // namespace xxx
