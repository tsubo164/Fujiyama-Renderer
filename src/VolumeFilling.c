/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "VolumeFilling.h"
#include "Numeric.h"
#include "Vector.h"
#include "Volume.h"

void LerpWispConstrolPoint(struct WispsControlPoint *cp,
		const struct WispsControlPoint *cp0, const struct WispsControlPoint *cp1,
		double t)
{
	VEC3_LERP(cp->orig, cp0->orig, cp1->orig, t);
	VEC3_LERP(cp->udir, cp0->udir, cp1->udir, t);
	VEC3_LERP(cp->vdir, cp0->vdir, cp1->vdir, t);
	VEC3_LERP(cp->wdir, cp0->wdir, cp1->wdir, t);
	VEC3_LERP(cp->noise_space, cp0->noise_space, cp1->noise_space, t);
	VEC3_NORMALIZE(cp->udir);
	VEC3_NORMALIZE(cp->vdir);
	VEC3_NORMALIZE(cp->wdir);

	cp->density = LERP(cp0->density, cp1->density, t);
	cp->radius = LERP(cp0->radius, cp1->radius, t);
	cp->noise_amplitude = LERP(cp0->noise_amplitude, cp1->noise_amplitude, t);
	/* TODO should not point attribute? */
	cp->speck_count = LERP(cp0->speck_count, cp1->speck_count, t);
	cp->speck_radius = LERP(cp0->speck_radius, cp1->speck_radius, t);
}

void FillWithSphere(struct Volume *volume,
		const double *center, double radius, float density)
{
	int i, j, k;
	int xmin, ymin, zmin;
	int xmax, ymax, zmax;
	const double thresholdwidth = .5 * VolGetFilterSize(volume);

	VolGetIndexRange(volume, center, radius,
			&xmin, &ymin, &zmin,
			&xmax, &ymax, &zmax);

	for (k = zmin; k <= zmax; k++) {
		for (j = ymin; j <= ymax; j++) {
			for (i = xmin; i <= xmax; i++) {
				double P[3] = {0};
				float value = 0;

				VolIndexToPoint(volume, i, j, k, P);

				P[0] -= center[0];
				P[1] -= center[1];
				P[2] -= center[2];

				value = VolGetValue(volume, i, j, k);
				value += density * Fit(VEC3_LEN(P) - radius,
						-thresholdwidth, thresholdwidth, 1, 0);
				VolSetValue(volume, i, j, k, value);
			}
		}
	}
}

