// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_POINTCLOUD_H
#define FJ_POINTCLOUD_H

#include "fj_compatibility.h"
#include "fj_primitive_set.h"
#include "fj_geometry.h"
#include "fj_vector.h"
#include "fj_types.h"
#include "fj_box.h"
#include <vector>

namespace fj {

class FJ_API PointCloud : public PrimitiveSet, public Geometry {
public:
  PointCloud();
  virtual ~PointCloud();

private:
  virtual bool ray_intersect(Index prim_id, const Ray &ray,
      Real time, Intersection *isect) const;
  virtual bool box_intersect(Index prim_id, const Box &box) const;
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const;
  virtual void get_bounds(Box *bounds) const;
  virtual Index get_primitive_count() const;

  virtual void compute_bounds();
};

} // namespace xxx

#endif // FJ_XXX_H
