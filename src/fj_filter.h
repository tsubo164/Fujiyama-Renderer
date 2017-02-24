// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_FILTER_H
#define FJ_FILTER_H

#include "fj_types.h"

namespace fj {

enum {
  FLT_BOX = 0,
  FLT_GAUSSIAN
};

class Filter {
public:
  Filter();
  ~Filter();

  void SetFilterType(int filtertype, Real xwidth, Real ywidth);
  Real Evaluate(Real x, Real y) const;

private:
  Real xwidth_, ywidth_;
  Real (*evaluate_)(Real xwidth, Real ywidth, Real x, Real y);
};

} // namespace xxx

#endif // FJ_XXX_H
