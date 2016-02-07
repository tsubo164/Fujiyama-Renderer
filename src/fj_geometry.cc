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

template<typename T> inline
bool out_of_range(const std::vector<T> &v, Index i)
{
  return i < 0 || i >= v.size();
}
template<typename T> inline
void add_attribute(std::vector<T> &v, Index size)
{
  v.resize(size);
}
template<typename T> inline
void set_attribute(std::vector<T> &v, Index i, const T &value)
{
  if (out_of_range(v, i)) {
    return;
  }
  v[i] = value;
}
template<typename T> inline
T get_attribute(const std::vector<T> &v, Index i)
{
  if (out_of_range(v, i)) {
    return T();
  }
  return v[i];
}
template<typename T> inline
bool has_attribute(const std::vector<T> &v)
{
  return !v.empty();
}

#define ATTRIBUTE_LIST(ATTR) \
  ATTR(Vector, Point, Position) \
  ATTR(Vector, Point, Velocity) \
  ATTR(Real,   Point, Radius)

#define ATTR(Type, Class, Name) \
void Geometry::Add##Class##Name() \
{ \
  add_attribute(Class##Name##_, Get##Class##Count()); \
} \
Type Geometry::Get##Class##Name(Index index) const \
{ \
  return get_attribute(Class##Name##_, index); \
} \
void Geometry::Set##Class##Name(Index index, const Type &value) \
{ \
  set_attribute(Class##Name##_, index, value); \
} \
bool Geometry::Has##Class##Name() const \
{ \
  return has_attribute(Class##Name##_); \
}
  ATTRIBUTE_LIST(ATTR)
#undef ATTR

} // namespace xxx
