// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_point_cloud.h"
#include "fj_intersection.h"
#include "fj_numeric.h"
#include "fj_ray.h"

namespace fj {

PointCloud::PointCloud()
{
}

PointCloud::~PointCloud()
{
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
  isect->N = Normalize(isect->N);

  isect->object = NULL;
  isect->prim_id = prim_id;
  isect->t_hit = t_hit;

  return true;
}

bool PointCloud::box_intersect(Index prim_id, const Box &box) const
{
  const Vector velocity = GetPointVelocity(prim_id);
  const Vector P0 = GetPointPosition(prim_id);
  const Real radius = GetPointRadius(prim_id);
  const int N_STEPS = 16;
  const Vector step = velocity / N_STEPS;

  for (int i = 0; i < N_STEPS; i++) {
    const Vector P = P0 + i * step;
    const Vector Q = P + step;
    Box segment_bounds(P, Q);
    segment_bounds.Expand(radius);

    if (BoxBoxIntersect(segment_bounds, box)) {
      return true;
    }
  }
  return false;
}

void PointCloud::get_primitive_bounds(Index prim_id, Box *bounds) const
{
  const Vector P = GetPointPosition(prim_id);
  const Vector velocity = GetPointVelocity(prim_id);
  const Real radius = GetPointRadius(prim_id);

  *bounds = Box(P, P);

  bounds->AddPoint(P + velocity);
  bounds->Expand(radius);
}
void PointCloud::get_bounds(Box *bounds) const
{
  *bounds = GetBounds();
}

Index PointCloud::get_primitive_count() const
{
  return GetPointCount();
}

void PointCloud::compute_bounds()
{
  Box box;
  box.ReverseInfinite(); 
  for (int i = 0; i < GetPointCount(); i++) {
    Box ptbox;
    GetPrimitiveBounds(i, &ptbox);
    box.AddBox(ptbox);
  }
  set_bounds(box);
}

} // namespace xxx
