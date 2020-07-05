// Copyright (c) 2011-2020 Hiroshi Tsubokawa
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

  bool RayIntersect(Index prim_id, const Ray &ray, Real time, Intersection *isect) const;
  bool BoxIntersect(Index prim_id, const Box &box) const;

  void GetPrimitiveBounds(Index prim_id, Box *bounds) const;
  void GetEntireBounds(Box *bounds) const;
  Index GetPrimitiveCount() const;

private:
  virtual bool ray_intersect(Index prim_id, const Ray &ray,
      Real time, Intersection *isect) const = 0;
  // TODO make this pure virtual
  virtual bool box_intersect(Index prim_id, const Box &box) const
  {
    return true;
  }
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const = 0;
  // TODO rename this
  virtual void get_bounds(Box *bounds) const = 0;
  virtual Index get_primitive_count() const = 0;
};

} // namespace xxx

#endif // FJ_XXX_H
