// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_GEOMETRY_H
#define FJ_GEOMETRY_H

#include "fj_compatibility.h"
#include "fj_vector.h"
#include "fj_types.h"
#include "fj_box.h"
#include <vector>

namespace fj {

class FJ_API Geometry {
public:
  Geometry();
  virtual ~Geometry();

  Index GetPointCount() const;
  void SetPointCount(Index point_count);

  const Box &GetBounds() const;
  void ComputeBounds();

  // Position
  void   AddPointPosition();
  Vector GetPointPosition(int index) const;
  void   SetPointPosition(int index, const Vector &value);
  bool   HasPointPosition() const;

  // Velocity
  void   AddPointVelocity();
  Vector GetPointVelocity(int index) const;
  void   SetPointVelocity(int index, const Vector &value);
  bool   HasPointVelocity() const;

  // Radius
  void   AddPointRadius();
  Real   GetPointRadius(int index) const;
  void   SetPointRadius(int index, const Real &value);
  bool   HasPointRadius() const;

protected:
  void set_bounds(const Box &bounds);

private:
  virtual void compute_bounds() = 0;

  Index point_count_;
  Box bounds_;

  std::vector<Vector> PointPosition_;
  std::vector<Vector> PointVelocity_;
  std::vector<Real>   PointRadius_;
};

} // namespace xxx

#endif // FJ_XXX_H
