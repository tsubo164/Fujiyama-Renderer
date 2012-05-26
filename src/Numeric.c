/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Numeric.h"
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

