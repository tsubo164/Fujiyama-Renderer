// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_TYPES_H
#define FJ_TYPES_H

#include "fj_compatibility.h"

#if defined(FJ_MACOSX) && !defined(__clang__)
  #include <cfloat>
  #define FJ_REAL_MAX DBL_MAX
#else
  #define FJ_REAL_MAX std::numeric_limits<fj::Real>::max()
#endif

namespace fj {

//XXX CHANGE FJ_REAL_MAX IF REAL IS FLOAT
typedef double Real;
typedef int Index;

class FJ_API Index3 {
public:
  Index3() : i0(0), i1(0), i2(0) {}
  Index3(Index ii0, Index ii1, Index ii2) : i0(ii0), i1(ii1), i2(ii2) {}
  ~Index3() {}

public:
  Index i0, i1, i2;
};

} // namespace xxx

#endif // FJ_XXX_H
