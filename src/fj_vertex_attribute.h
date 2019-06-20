// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_VERTEX_ATTRIBUTE_H
#define FJ_VERTEX_ATTRIBUTE_H

#include "fj_compatibility.h"
#include "fj_types.h"

#include <vector>

namespace fj {

template<typename T>
class FJ_API VertexAttribute {
public:
  typedef T Value;

public:
  VertexAttribute() {}
  ~VertexAttribute() {}

  bool IsEmpty() const
  {
    return value_.empty();
  }

  // Resize
  void ResizeValue(Index size)
  {
    value_.resize(size);
  }
  void ResizeIndex(Index size)
  {
    index_.resize(size);
  }

  // Get
  Value GetValue(Index offset) const
  {
    if (out_of_range(value_, offset)) {
      return Value();
    }
    return value_[offset];
  }
  Index GetIndex(Index offset) const
  {
    if (out_of_range(index_, offset)) {
      return Index();
    }
    return index_[offset];
  }

  // Set
  void SetValue(Index offset, const Value &value)
  {
    if (out_of_range(value_, offset)) {
      return;
    }
    value_[offset] = value;
  }
  void SetIndex(Index offset, Index index)
  {
    if (out_of_range(index_, offset)) {
      return;
    }
    index_[offset] = index;
  }

  // Push
  void PushValue(const Value &value)
  {
    value_.push_back(value);
  }
  void PushIndex(const Index &index)
  {
    index_.push_back(index);
  }

  // Count
  Index GetValueCount() const
  {
    return value_.size();
  }
  Index GetIndexCount() const
  {
    return index_.size();
  }

  // TODO better name like Lookup
  Value Get(Index index) const
  {
    if (out_of_range(index_, index)) {
      return Value();
    }
    return value_[index_[index]];
  }

  void Clear()
  {
    std::vector<Value>().swap(value_);
    std::vector<Index>().swap(index_);
  }

private:
  template <typename T2>
  bool out_of_range(const std::vector<T2> &v, std::size_t i) const
  {
    return i < 0 || i >= v.size();
  }

  std::vector<Value> value_;
  std::vector<Index> index_;
};

} // namespace xxx

#endif // FJ_XXX_H
