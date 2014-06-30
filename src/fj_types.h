// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_TYPES_H
#define FJ_TYPES_H

#include "fj_compatibility.h"

namespace fj {

typedef double Real;
typedef int Index;

class FJ_API Index3 {
public:
  Index3() : i0(0), i1(0), i2(0) {}
  ~Index3() {}

public:
  Index i0, i1, i2;
};

} // namespace xxx

#endif // FJ_XXX_H
