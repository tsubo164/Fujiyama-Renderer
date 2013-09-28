/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_numeric.h"
#include <math.h>

double SmoothStep(double x, double a, double b)
{
  const double t = (x-a) / (b-a);

  if (t <= 0)
    return 0;

  if (t >= 1)
    return 1;

  return t*t*(3 - 2*t);
}

double Gamma(double x, double g)
{
  return pow(x, g);
}

double Fit(double x, double src0, double src1, double dst0, double dst1)
{
  if (x <= src0)
    return dst0;

  if (x >= src1)
    return dst1;

  return dst0 + (dst1 - dst0) * ((x - src0) / (src1 - src0));
}

double Bilerp(double v00, double v10, double v01, double v11, double s, double t)
{
  const double a = LERP(v00, v01, t);
  const double b = LERP(v10, v11, t);
  return LERP(a, b, s);
}

