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

void Geometry::ComputeBounds()
{
  compute_bounds();
}

void Geometry::set_bounds(const Box &bounds)
{
  bounds_ = bounds;
}

} // namespace xxx
