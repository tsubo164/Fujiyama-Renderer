/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_NUMERIC_H
#define FJ_NUMERIC_H

#include "fj_types.h"
#include <limits>
#include <cmath>

namespace fj {

const Real PI = 3.14159265358979323846;
const Real REAL_MAX = std::numeric_limits<Real>::max();

inline Real Abs(Real x)
{
  return x < 0 ? -x : x;
}

inline Real Min(Real x, Real y)
{
  return x < y ? x : y;
}

inline Real Max(Real x, Real y)
{
  return x > y ? x : y;
}

inline Real Clamp(Real x, Real a, Real b)
{
  return x < a ? a : (x > b ? b : x);
}

inline Real Radian(Real deg)
{
  return deg * PI / 180.;
}

inline Real Lerp(Real a, Real b, Real t)
{
  return (1 - t) * a + t * b;
}

inline Real SmoothStep(Real x, Real a, Real b)
{
  const Real t = (x-a) / (b-a);

  if (t <= 0)
    return 0;

  if (t >= 1)
    return 1;

  return t * t * (3 - 2 * t);
}

inline Real Gamma(Real x, Real g)
{
  return std::pow(x, g);
}

inline Real Fit(Real x, Real src0, Real src1, Real dst0, Real dst1)
{
  if (x <= src0)
    return dst0;

  if (x >= src1)
    return dst1;

  return dst0 + (dst1 - dst0) * ((x - src0) / (src1 - src0));
}

inline Real Bilerp(Real v00, Real v10, Real v01, Real v11, Real s, Real t)
{
  const Real a = Lerp(v00, v01, t);
  const Real b = Lerp(v10, v11, t);
  return Lerp(a, b, s);
}

} // namespace xxx

#endif /* FJ_XXX_H */
