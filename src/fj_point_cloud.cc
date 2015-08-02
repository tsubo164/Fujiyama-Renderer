// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_point_cloud.h"
#include "fj_intersection.h"
#include "fj_numeric.h"
#include "fj_ray.h"

#define ATTRIBUTE_LIST(ATTR) \
  ATTR(Point, Vector,   P_,        Position) \
  ATTR(Point, Vector,   velocity_, Velocity) \
  ATTR(Point, Real,     radius_,   Radius)

namespace fj {

#define ATTR(Class, Type, Name, Label) \
void PointCloud::Add##Class##Label() \
{ \
  Name.resize(Get##Class##Count()); \
} \
Type PointCloud::Get##Class##Label(int idx) const \
{ \
  if (idx < 0 || idx >= static_cast<int>(Name.size())) { \
    return Type(); \
  } \
  return Name[idx]; \
} \
void PointCloud::Set##Class##Label(int idx, const Type &value) \
{ \
  if (idx < 0 || idx >= static_cast<int>(Name.size())) \
    return; \
  Name[idx] = value; \
} \
bool PointCloud::Has##Class##Label() const \
{ \
  return !Name.empty(); \
}
  ATTRIBUTE_LIST(ATTR)
#undef ATTR

PointCloud::PointCloud() : point_count_(0)
{
}

PointCloud::~PointCloud()
{
}

int PointCloud::GetPointCount() const
{
  return point_count_;
}

void PointCloud::SetPointCount(int point_count)
{
  point_count_ = point_count;
}

const Box &PointCloud::GetBounds() const
{
  return bounds_;
}

void PointCloud::ComputeBounds()
{
  BoxReverseInfinite(&bounds_); 

  for (int i = 0; i < GetPointCount(); i++) {
    Box ptbox;
    GetPrimitiveBounds(i, &ptbox);
    BoxAddBox(&bounds_, ptbox);
  }
}

bool PointCloud::ray_intersect(Index prim_id, const Ray &ray,
    Real time, Intersection *isect) const
{
/*
  X = o + t * d;
  (X - center) * (X - center) = R * R;
  |d|^2 * t^2 + 2 * d * (o - center) * t + |o - center|^2 - r^2 = 0;

  t = (-d * (o - center) +- sqrt(D)) / |d|^2;
  D = {d * (o - center)}^2 - |d|^2 * (|o - center|^2 - r^2);
*/
  const Vector P = GetPointPosition(prim_id);
  const Vector velocity = GetPointVelocity(prim_id);
  const Real radius = GetPointRadius(prim_id);

  const Vector center = P + time * velocity;
  const Vector orig_local = ray.orig - center;

  const Real a = Dot(ray.dir, ray.dir);
  const Real b = Dot(ray.dir, orig_local);
  const Real c = Dot(orig_local, orig_local) - radius * radius;

  const Real discriminant = b * b - a * c;
  if (discriminant < 0.) {
    return false;
  }

  const Real disc_sqrt = sqrt(discriminant);
  const Real t0 = -b - disc_sqrt;
  const Real t1 = -b + disc_sqrt;

  if (t1 <= 0.) {
    return false;
  }
  const Real t_hit = (t1 <= 0.) ? t1 : t0;

  isect->P = RayPointAt(ray, t_hit);
  isect->N = isect->P - center;
  Normalize(&isect->N);

  isect->object = NULL;
  isect->prim_id = prim_id;
  isect->t_hit = t_hit;

  return true;
}

bool PointCloud::box_intersect(Index prim_id, const Box &box) const
{
  // TODO support velocity
  /* On Faster Sphere-Box Overlap Testing
   * Thomas Larsson, Tomas Akenine-Möller & Eric Lengyel
   */
  const Vector center = GetPointPosition(prim_id);
  const Real radius = GetPointRadius(prim_id);
  const Vector &min = box.min;
  const Vector &max = box.max;
  Real d = 0;
  Real e = 0;

  for (int i = 0; i < 3; i++) {
    if ((e = center[i] - min[i]) < 0.) {
      if (e < -radius) {
        return false;
      } else {
        d = d + e * e;
      }
    } else if ((e = center[i] - max[i]) > 0.) {
      if (e > radius) {
        return false;
      } else {
        d = d + e * e;
      }
    }
  }
  if (d <= radius) {
    return true;
  }
  return false;
}

void PointCloud::get_primitive_bounds(Index prim_id, Box *bounds) const
{
  const Vector P = GetPointPosition(prim_id);
  const Vector velocity = GetPointVelocity(prim_id);
  const Real radius = GetPointRadius(prim_id);

  *bounds = Box(
      P.x, P.y, P.z,
      P.x, P.y, P.z);

  BoxAddPoint(bounds, P + velocity);
  BoxExpand(bounds, radius);
}
void PointCloud::get_bounds(Box *bounds) const
{
  *bounds = GetBounds();
}

Index PointCloud::get_primitive_count() const
{
  return GetPointCount();
}

} // namespace xxx
