/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Numeric.h"

double SmoothStep(double x, double a, double b)
{
	const double t = (x-a) / (b-a);

	if (t <= 0)
		return 0;

	if (t >= 1)
		return 1;

	return t*t*(3 - 2*t);
}

