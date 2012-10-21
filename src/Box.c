/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Box.h"
#include "Numeric.h"
#include <stdio.h>
#include <math.h>

int BoxContainsPoint(const double *box, const double *point)
{
	if ((point[0] < box[0]) || (point[0] > box[3])) return 0;
	if ((point[1] < box[1]) || (point[1] > box[4])) return 0;
	if ((point[2] < box[2]) || (point[2] > box[5])) return 0;

	return 1;
}

void BoxAddPoint(double *box, const double *point)
{
		box[0] = MIN(box[0], point[0]);
		box[1] = MIN(box[1], point[1]);
		box[2] = MIN(box[2], point[2]);
		box[3] = MAX(box[3], point[0]);
		box[4] = MAX(box[4], point[1]);
		box[5] = MAX(box[5], point[2]);
}

void BoxAddBox(double *box, const double *otherbox)
{
		box[0] = MIN(box[0], otherbox[0]);
		box[1] = MIN(box[1], otherbox[1]);
		box[2] = MIN(box[2], otherbox[2]);
		box[3] = MAX(box[3], otherbox[3]);
		box[4] = MAX(box[4], otherbox[4]);
		box[5] = MAX(box[5], otherbox[5]);
}

int BoxRayIntersect(const double *box,
		const double *rayorig, const double *raydir,
		double ray_tmin, double ray_tmax,
		double *hit_tmin, double *hit_tmax)
{
	int hit;
	double tmin, tmax, tymin, tymax, tzmin, tzmax;

	if (raydir[0] >= 0) {
		tmin = (box[0] - rayorig[0]) / raydir[0];
		tmax = (box[3] - rayorig[0]) / raydir[0];
	} else {
		tmin = (box[3] - rayorig[0]) / raydir[0];
		tmax = (box[0] - rayorig[0]) / raydir[0];
	}

	if (raydir[1] >= 0) {
		tymin = (box[1] - rayorig[1]) / raydir[1];
		tymax = (box[4] - rayorig[1]) / raydir[1];
	} else {
		tymin = (box[4] - rayorig[1]) / raydir[1];
		tymax = (box[1] - rayorig[1]) / raydir[1];
	}

	if ((tmin > tymax) || (tymin > tmax))
		return 0;

	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	if (raydir[2] >= 0) {
		tzmin = (box[2] - rayorig[2]) / raydir[2];
		tzmax = (box[5] - rayorig[2]) / raydir[2];
	} else {
		tzmin = (box[5] - rayorig[2]) / raydir[2];
		tzmax = (box[2] - rayorig[2]) / raydir[2];
	}

	if ((tmin > tzmax) || (tzmin > tmax))
		return 0;

	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	hit = ((tmin < ray_tmax) && (tmax > ray_tmin));
	if (hit) {
		*hit_tmin = tmin;
		*hit_tmax = tmax;
	}

	return hit;
}

void BoxCentroid(const double *box, double *centroid)
{
	centroid[0] = .5 * (box[0] + box[3]);
	centroid[1] = .5 * (box[1] + box[4]);
	centroid[2] = .5 * (box[2] + box[5]);
}

double BoxDiagonal(const double *box)
{
	const double xsize = BOX3_XSIZE(box);
	const double ysize = BOX3_YSIZE(box);
	const double zsize = BOX3_ZSIZE(box);

	return .5 * sqrt(xsize * xsize + ysize * ysize + zsize * zsize);
}

void BoxPrint(const double *box)
{
	printf("(%g, %g, %g) (%g, %g, %g)\n",
			box[0], box[1], box[2],
			box[3], box[4], box[5]);
}

