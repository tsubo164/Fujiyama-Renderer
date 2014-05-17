/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_point_cloud.h"
#include "fj_intersection.h"
#include "fj_primitive_set.h"
#include "fj_numeric.h"
#include "fj_memory.h"
#include "fj_ray.h"

#include <cstring>
#include <cfloat>

#define ATTRIBUTE_LIST(ATTR) \
  ATTR(Point, Vector,   P_,        Position) \
  ATTR(Point, Vector,   velocity_, Velocity) \
  ATTR(Point, Real,     radius_,   Radius)

namespace fj {

static int point_ray_intersect(const void *prim_set, int prim_id, double time,
    const Ray *ray, Intersection *isect);
static void point_bounds(const void *prim_set, int prim_id, Box *bounds);
static void point_cloud_bounds(const void *prim_set, Box *bounds);
static int point_count(const void *prim_set);

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
    point_bounds(this, i, &ptbox);
    BoxAddBox(&bounds_, ptbox);
  }
}

#define ATTR(Class, Type, Name, Label) \
void PointCloud::Add##Class##Label() \
{ \
  Name.resize(Get##Class##Count()); \
} \
Type PointCloud::Get##Class##Label(int idx) const \
{ \
  if (idx < 0 || idx >= static_cast<int>(this->Name.size())) { \
    return Type(); \
  } \
  return this->Name[idx]; \
} \
void PointCloud::Set##Class##Label(int idx, const Type &value) \
{ \
  if (idx < 0 || idx >= static_cast<int>(this->Name.size())) \
    return; \
  this->Name[idx] = value; \
} \
bool PointCloud::Has##Class##Label() const \
{ \
  return this->Name.size() > 0; \
}
  ATTRIBUTE_LIST(ATTR)
#undef ATTR

PointCloud *PtcNew(void)
{
  return new PointCloud();
}

void PtcFree(PointCloud *ptc)
{
  delete ptc;
}

void PtcGetPrimitiveSet(const PointCloud *ptc, PrimitiveSet *primset)
{
  MakePrimitiveSet(primset,
      "PointCloud",
      ptc,
      point_ray_intersect,
      point_bounds,
      point_cloud_bounds,
      point_count);
}

static int point_ray_intersect(const void *prim_set, int prim_id, double time,
    const Ray *ray, Intersection *isect)
{
/*
  X = o + t * d;
  (X - center) * (X - center) = R * R;
  |d|^2 * t^2 + 2 * d * (o - center) * t + |o - center|^2 - r^2 = 0;

  t = (-d * (o - center) +- sqrt(D)) / |d|^2;
  D = {d * (o - center)}^2 - |d|^2 * (|o - center|^2 - r^2);
*/
  const PointCloud *ptc = (const PointCloud *) prim_set;
  const Vector P = ptc->GetPointPosition(prim_id);
  const Vector velocity = ptc->GetPointVelocity(prim_id);
  const double radius = ptc->GetPointRadius(prim_id);

  const Vector center = ptc->HasPointVelocity() ? P + time * velocity : P;
  const Vector orig_local = ray->orig - center;

  const Real a = Dot(ray->dir, ray->dir);
  const Real b = Dot(ray->dir, orig_local);
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

  isect->P = RayPointAt(*ray, t_hit);
  isect->N = Normalize(isect->P - center);

  isect->object = NULL;
  isect->prim_id = prim_id;
  isect->t_hit = t_hit;

  return true;
}

static void point_bounds(const void *prim_set, int prim_id, Box *bounds)
{
  const PointCloud *ptc = (const PointCloud *) prim_set;
  const Vector P = ptc->GetPointPosition(prim_id);
  const Vector velocity = ptc->GetPointVelocity(prim_id);
  const Real radius = ptc->GetPointRadius(prim_id);

  *bounds = Box(
      P.x, P.y, P.z,
      P.x, P.y, P.z);

  if (ptc->HasPointVelocity()) {
    const Vector P_close = P + velocity;
    BoxAddPoint(bounds, P_close);
  }

  BoxExpand(bounds, radius);
}

static void point_cloud_bounds(const void *prim_set, Box *bounds)
{
  const PointCloud *ptc = (const PointCloud *) prim_set;
  *bounds = ptc->GetBounds();
}

static int point_count(const void *prim_set)
{
  const PointCloud *ptc = (const PointCloud *) prim_set;
  return ptc->GetPointCount();
}

} // namespace xxx
