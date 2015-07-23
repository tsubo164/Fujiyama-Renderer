// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_POINTCLOUD_H
#define FJ_POINTCLOUD_H

#include "fj_compatibility.h"
#include "fj_primitive_set.h"
#include "fj_vector.h"
#include "fj_types.h"
#include "fj_box.h"
#include <vector>

namespace fj {

class FJ_API PointCloud : public PrimitiveSet {
public:
  PointCloud();
  virtual ~PointCloud();

  int GetPointCount() const;
  void SetPointCount(int point_count);
  const Box &GetBounds() const;

  void AddPointPosition();
  void AddPointVelocity();
  void AddPointRadius();

  Vector   GetPointPosition(int idx) const;
  Vector   GetPointVelocity(int idx) const;
  Real     GetPointRadius(int idx) const;

  void SetPointPosition(int idx, const Vector &value);
  void SetPointVelocity(int idx, const Vector &value);
  void SetPointRadius(int idx, const Real &value);

  bool HasPointPosition() const;
  bool HasPointVelocity() const;
  bool HasPointRadius() const;

  void ComputeBounds();

private:
  virtual bool ray_intersect(Index prim_id, const Ray &ray,
      Real time, Intersection *isect) const;
  virtual bool box_intersect(Index prim_id, const Box &box) const;
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const;
  virtual void get_bounds(Box *bounds) const;
  virtual Index get_primitive_count() const;

  int point_count_;
  std::vector<Vector> P_;
  std::vector<Vector> velocity_;
  std::vector<Real> radius_;
  Box bounds_;
};

} // namespace xxx

#endif // FJ_XXX_H
