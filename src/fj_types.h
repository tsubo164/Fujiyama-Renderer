// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_TYPES_H
#define FJ_TYPES_H

#include "fj_compatibility.h"
#include <cassert>

namespace fj {

//XXX CHANGE FJ_REAL_MAX IF REAL IS FLOAT
using Real = double;
using Index = int;

class FJ_API Index3 {
public:
  Index3() : i0(0), i1(0), i2(0) {}
  Index3(Index ii0, Index ii1, Index ii2) : i0(ii0), i1(ii1), i2(ii2) {}
  ~Index3() {}

  Index operator[](int i) const
  {
    switch(i) {
    case 0: return i0;
    case 1: return i1;
    case 2: return i2;
    default:
      assert(!"bounds error at Index3::get");
      return i0;
    }
  }
  Index &operator[](int i)
  {
    switch(i) {
    case 0: return i0;
    case 1: return i1;
    case 2: return i2;
    default:
      assert(!"bounds error at Index3::set");
      return i0;
    }
  }

public:
  Index i0, i1, i2;
};

} // namespace xxx

#endif // FJ_XXX_H
