// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_PRIMITIVE_SET_H
#define FJ_PRIMITIVE_SET_H

#include "fj_compatibility.h"
#include "fj_types.h"

namespace fj {

class Intersection;
class Box;
class Ray;

// PrimitiveSet abstract a set of primitives that is used by Accelerator
class FJ_API PrimitiveSet {
public:
  PrimitiveSet() {}
  virtual ~PrimitiveSet() {}

  bool RayIntersect(Index prim_id, Real time, const Ray &ray, Intersection *isect) const
  {
    return ray_intersect(prim_id, time, ray, isect);
  }

  void GetPrimitiveBounds(Index prim_id, Box *bounds) const
  {
    get_primitive_bounds(prim_id, bounds);
  }

  void GetBounds(Box *bounds) const
  {
    get_bounds(bounds);
  }

  Index GetPrimitiveCount() const
  {
    return get_primitive_count();
  }

private:
  virtual bool ray_intersect(Index prim_id, Real time,
      const Ray &ray, Intersection *isect) const = 0;
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const = 0;
  virtual void get_bounds(Box *bounds) const = 0;
  virtual Index get_primitive_count() const = 0;
};

} // namespace xxx

#endif // FJ_XXX_H
