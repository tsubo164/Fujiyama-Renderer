// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_GEOMETRY_H
#define FJ_GEOMETRY_H

#include "fj_compatibility.h"
#include "fj_vector.h"
#include "fj_types.h"
#include "fj_box.h"
#include <vector>

namespace fj {

template<typename T>
class FJ_API Attribute {
public:
  typedef T Value;

public:
  Attribute() {}
  ~Attribute() {}

  bool IsEmpty() const
  {
    return value_.empty();
  }
  void Resize(Index size)
  {
    value_.resize(size);
  }
  Value Get(Index i) const
  {
    if (out_of_range(i)) {
      return Value();
    }
    return value_[i];
  }
  void Set(Index i, const Value &value)
  {
    if (out_of_range(i)) {
      return;
    }
    value_[i] = value;
  }
  void Push(const Value &value)
  {
    value_.push_back(value);
  }
  Index Count() const
  {
    return value_.size();
  }
  void Clear()
  {
    std::vector<Value>().swap(value_);
  }
private:
  bool out_of_range(std::size_t i) const
  {
    return i < 0 || i >= value_.size();
  }
  std::vector<Value> value_;
};

typedef Attribute<Vector> AttributeVector;
typedef Attribute<Index>  AttributeIndex;
typedef Attribute<Real>   AttributeReal;

class FJ_API Geometry {
public:
  Geometry();
  virtual ~Geometry();

  Index GetPointCount() const;
  void SetPointCount(Index point_count);
  const Box &GetBounds() const;
  void SetBounds(const Box &bounds);

  const AttributeVector &PointPosition() const { return pt_position_; };
  AttributeVector &PointPosition()             { return pt_position_; };
  const AttributeVector &PointNormal() const { return pt_normal_; };
  AttributeVector &PointNormal()             { return pt_normal_; };
  const AttributeVector &PointColor() const { return pt_color_; };
  AttributeVector &PointColor()             { return pt_color_; };
  const AttributeVector &PointVelocity() const { return pt_velocity_; };
  AttributeVector &PointVelocity()             { return pt_velocity_; };
  const AttributeReal &PointRadius() const { return pt_radius_; };
  AttributeReal &PointRadius()             { return pt_radius_; };

private:
  Index point_count_;
  Box bounds_;

  AttributeVector pt_position_;
  AttributeVector pt_normal_;
  AttributeVector pt_color_;
  AttributeVector pt_velocity_;
  AttributeReal   pt_radius_;

  AttributeIndex  pr_index_;
};

} // namespace xxx

#endif // FJ_XXX_H
