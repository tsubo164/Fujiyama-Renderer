/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

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

public:
  Real xwidth_, ywidth_;
  Real (*evaluate_)(Real xwidth, Real ywidth, Real x, Real y);
};

extern Filter *FltNew(int filtertype, Real xwidth, Real ywidth);
extern void FltFree(Filter *filter);

} // namespace xxx

#endif // FJ_XXX_H
