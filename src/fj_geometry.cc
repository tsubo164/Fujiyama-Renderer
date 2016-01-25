// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_geometry.h"

namespace fj {

Geometry::Geometry() : point_count_(0)
{
}

Geometry::~Geometry()
{
}

Index Geometry::GetPointCount() const
{
  return point_count_;
}

void Geometry::SetPointCount(Index point_count)
{
  point_count_ = point_count;
}

const Box &Geometry::GetBounds() const
{
  return bounds_;
}

void Geometry::SetBounds(const Box &bounds)
{
  bounds_ = bounds;
}

  /*
Vector Geometry::GetPointPosition(Index idx) const
{
  const Attribute<Vector> &P = GetPointPosition();
  return P.Get(idx);
}
void Geometry::SetPointPosition(int idx, const Vector &value)
{
  Attribute<Vector> &P = GetPointPosition();
  P.SetValue(idx, value);
}
  */

/*
void Geometry::ComputeBounds()
{
  bounds_.ReverseInfinite(); 

  for (int i = 0; i < GetPointCount(); i++) {
    Box ptbox;
    GetPrimitiveBounds(i, &ptbox);
    bounds_.AddBox(ptbox);
  }
}
*/

} // namespace xxx
