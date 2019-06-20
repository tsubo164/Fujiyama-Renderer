// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_primitive_set.h"
#include "fj_intersection.h"
#include "fj_ray.h"

namespace fj {

bool PrimitiveSet::RayIntersect(Index prim_id, const Ray &ray,
    Real time, Intersection *isect) const
{
  const bool hit = ray_intersect(prim_id, ray, time, isect);

  if (!hit) {
    isect->t_hit = REAL_MAX;
    return false;
  }

  if (!RayInRange(ray, isect->t_hit)) {
    isect->t_hit = REAL_MAX;
    return false;
  }

  return true;
}

bool PrimitiveSet::BoxIntersect(Index prim_id, const Box &box) const
{
  return box_intersect(prim_id, box);
}

void PrimitiveSet::GetPrimitiveBounds(Index prim_id, Box *bounds) const
{
  get_primitive_bounds(prim_id, bounds);
}

void PrimitiveSet::GetEntireBounds(Box *bounds) const
{
  get_bounds(bounds);
}

Index PrimitiveSet::GetPrimitiveCount() const
{
  return get_primitive_count();
}

} // namespace xxx
