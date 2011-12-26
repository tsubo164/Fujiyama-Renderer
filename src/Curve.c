/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Curve.h"
#include "LocalGeometry.h"
#include "Accelerator.h"
#include "Matrix.h"
#include "Vector.h"
#include "Ray.h"
#include "Box.h"
#include <stdlib.h>
#include <stdio.h>

static int curve_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);
static void curve_bounds(const void *prim_set, int prim_id, double *bounds);

static void world_to_ray_matrix(const struct Ray *ray, struct Matrix *m);

struct Curve {
	double position[12];

	double bounds[6];
	int ncurves;
};

struct Curve *CrvNew(void)
{
	struct Curve *curve;

	curve = (struct Curve *) malloc(sizeof(struct Curve));
	if (curve == NULL)
		return NULL;

	VEC3_SET(&curve->position[0], -1, 0, 0);
	VEC3_SET(&curve->position[3],  1, 1, 0);
	VEC3_SET(&curve->position[6], -1, 2, 0);
	VEC3_SET(&curve->position[9],  1, 3, 0);

	BOX3_SET(curve->bounds, -1, 0, 0, 1, 3, 0);
	BOX3_EXPAND(curve->bounds, .1);
	curve->ncurves = 1;

	puts("Curve new");

	return curve;
}

void CrvFree(struct Curve *curve)
{
	puts("Curve free");
	if (curve == NULL)
		return;
	free(curve);
}

void CrvSetupAccelerator(const struct Curve *curve, struct Accelerator *acc)
{
	AccSetTargetGeometry(acc,
			curve,
			curve->ncurves,
			curve->bounds,
			curve_ray_intersect,
			curve_bounds);
}

static int curve_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	double half_width = .2 * .5;
	double half_width_sq = half_width * half_width;

	struct Matrix world_to_ray;
	int depth;

	depth = 4;

	return 0;
}

static void curve_bounds(const void *prim_set, int prim_id, double *bounds)
{
	const struct Curve *curve = (const struct Curve *) prim_set;
	BOX3_COPY(bounds, curve->bounds);
}

static void world_to_ray_matrix(const struct Ray *ray, struct Matrix *m)
{
	double d, d_inv;

	d = sqrt(ray->dir[0] * ray->dir[0] + ray->dir[2] * ray->dir[2]);
	d_inv = 1. / d;
}

#if 0
void CrvSetPosition(struct Curve *curve, double xpos, double ypos, double zpos)
{
	curve->position[0] = xpos;
	curve->position[1] = ypos;
	curve->position[2] = zpos;
}

void CrvSetColor(struct Curve *curve, float r, float g, float b)
{
	curve->color[0] = r;
	curve->color[1] = g;
	curve->color[2] = b;
}

void CrvSetIntensity(struct Curve *curve, double intensity)
{
	curve->intensity = intensity;
}

const double *CrvGetPosition(const struct Curve *curve)
{
	return curve->position;
}

void CrvIlluminate(const struct Curve *curve, const double *Ps, float *Cl)
{
	Cl[0] = curve->intensity * curve->color[0];
	Cl[1] = curve->intensity * curve->color[1];
	Cl[2] = curve->intensity * curve->color[2];
}
#endif

