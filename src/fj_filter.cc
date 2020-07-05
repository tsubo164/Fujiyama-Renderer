// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_filter.h"
#include <cassert>
#include <cmath>

namespace fj {

static Real eval_gaussian(Real xwidth, Real ywidth, Real x, Real y);
static Real eval_box(Real xwidth, Real ywidth, Real x, Real y);

Filter::Filter() :
    xwidth_(1),
    ywidth_(1),
    evaluate_(eval_box)
{
}

Filter::~Filter()
{
}

void Filter::SetFilterType(int filtertype, Real xwidth, Real ywidth)
{
  assert(xwidth > 0);
  assert(ywidth > 0);

  switch (filtertype) {
  case FLT_GAUSSIAN:
    evaluate_ = eval_gaussian;
    break;
  case FLT_BOX:
    evaluate_ = eval_box;
    break;
  default:
    assert(!"invalid filter type");
    break;
  }
  xwidth_ = xwidth;
  ywidth_ = ywidth;
}

Real Filter::Evaluate(Real x, Real y) const
{
  return evaluate_(xwidth_, ywidth_, x, y);
}

static Real eval_gaussian(Real xwidth, Real ywidth, Real x, Real y)
{
  // The RenderMan Interface
  // Version 3.2.1
  // November, 2005
  const Real xx = 2 * x / xwidth;
  const Real yy = 2 * y / ywidth;

  return exp(-2 * ( xx * xx + yy * yy ));
}

static Real eval_box(Real xwidth, Real ywidth, Real x, Real y)
{
  return 1;
}

} // namespace xxx
