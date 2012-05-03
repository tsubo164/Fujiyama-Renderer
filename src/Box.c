/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Box.h"
#include "Numeric.h"
#include <stdio.h>

void BoxSet(struct Box *box,
		double xmin, double ymin, double zmin,
		double xmax, double ymax, double zmax)
{
	box->min[0] = xmin;
	box->min[1] = ymin;
	box->min[2] = zmin;
	box->max[0] = xmax;
	box->max[1] = ymax;
	box->max[2] = zmax;
}

void BoxExpand(struct Box *box, double delta)
{
	box->min[0] -= delta;
	box->min[1] -= delta;
	box->min[2] -= delta;
	box->max[0] += delta;
	box->max[1] += delta;
	box->max[2] += delta;
}

void BoxPrint(const struct Box *box)
{
	printf("(%g, %g, %g) (%g, %g, %g)\n",
		box->min[0], box->min[1], box->min[2],
		box->max[0], box->max[1], box->max[2]);
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

	/*
	return ((tmin < ray_tmax) && (tmax > ray_tmin));
	*/

	hit = ((tmin < ray_tmax) && (tmax > ray_tmin));
	if (hit) {
		*hit_tmin = tmin;
		*hit_tmax = tmax;
	}

	return hit;
}

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

/* TODO ========================================== */
#include "Ray.h"
int BoxRayIntersect2(const struct Box *box, const struct Ray *ray, struct BoxHit *boxhit)
{
	int hit;
	double tmin, tmax, tymin, tymax, tzmin, tzmax;

	if (ray->dir[0] >= 0) {
		tmin = (box->min[0] - ray->orig[0]) / ray->dir[0];
		tmax = (box->max[0] - ray->orig[0]) / ray->dir[0];
	} else {
		tmin = (box->max[0] - ray->orig[0]) / ray->dir[0];
		tmax = (box->min[0] - ray->orig[0]) / ray->dir[0];
	}

	if (ray->dir[1] >= 0) {
		tymin = (box->min[1] - ray->orig[1]) / ray->dir[1];
		tymax = (box->max[1] - ray->orig[1]) / ray->dir[1];
	} else {
		tymin = (box->max[1] - ray->orig[1]) / ray->dir[1];
		tymax = (box->min[1] - ray->orig[1]) / ray->dir[1];
	}

	if ((tmin > tymax) || (tymin > tmax))
		return 0;

	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	if (ray->dir[2] >= 0) {
		tzmin = (box->min[2] - ray->orig[2]) / ray->dir[2];
		tzmax = (box->max[2] - ray->orig[2]) / ray->dir[2];
	} else {
		tzmin = (box->max[2] - ray->orig[2]) / ray->dir[2];
		tzmax = (box->min[2] - ray->orig[2]) / ray->dir[2];
	}

	if ((tmin > tzmax) || (tzmin > tmax))
		return 0;

	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	hit = ((tmin < ray->tmax) && (tmax > ray->tmin));
	if (hit) {
		boxhit->tmin = tmin;
		boxhit->tmax = tmax;
	}

	return hit;
}

