// Copyright (c) 2011-2014 Hiroshi Tsubokawa
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

  void ResizeValue(Index size)
  {
    value_.resize(size);
  }
  void ResizeIndex(Index size)
  {
    index_.resize(size);
  }
  void Resize(Index value_count, Index index_count)
  {
    value_.resize(value_count);
    index_.resize(index_count);
  }

  Value GetValue(Index offset) const { return value_[offset]; }
  Index GetIndex(Index offset) const { return index_[offset]; }
  void SetValue(Index offset, const Value &value)
  {
    value_[offset] = value;
  }
  void SetIndex(Index offset, Index index)
  {
    index_[offset] = index;
  }

  void PushValue(const Value &value)
  {
    value_.push_back(value);
  }

  Index GetValueCount() const
  {
    return value_.size();
  }
  Index GetIndexCount() const
  {
    return index_.size();
  }

  // TODO
  Value Get(Index index) const
  {
    if (index < 0 || index >= static_cast<Index>(index_.size())) {
      return Value();
    }
    return value_[index_[index]];
  }

  void Clear()
  {
    std::vector<Value>().swap(value_);
    std::vector<Index>().swap(index_);
  }

  const T &operator[](Index i) const
  {
    return value_[index_[i]];
  }
  T &operator[](Index i)
  {
    return value_[index_[i]];
  }

private:
  std::vector<Value> value_;
  std::vector<Index> index_;
};

} // namespace xxx

#endif // FJ_XXX_H
