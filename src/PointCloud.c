/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "PointCloud.h"
#include "Intersection.h"
#include "PrimitiveSet.h"
#include "Numeric.h"
#include "Memory.h"
#include "Vector.h"
#include "Box.h"
#include "Ray.h"

#include <float.h>

static int point_ray_intersect(const void *prim_set, int prim_id, double time,
		const struct Ray *ray, struct Intersection *isect);
static void point_bounds(const void *prim_set, int prim_id, struct Box *bounds);
static void point_cloud_bounds(const void *prim_set, struct Box *bounds);
static int point_count(const void *prim_set);

static void update_bounds(struct PointCloud *ptc);

struct PointCloud {
	int point_count;
	struct Vector *P;
	double *radius;

	struct Box bounds;
};

struct PointCloud *PtcNew(void)
{
	struct PointCloud *ptc = NULL;

	ptc = MEM_ALLOC(struct PointCloud);
	if (ptc == NULL)
		return NULL;

	ptc->point_count = 0;
	ptc->P = NULL;
	BOX3_SET(&ptc->bounds, 0, 0, 0, 0, 0, 0);

	/* TODO TEST */
#if 0
	PtcAllocatePoint(ptc, 1);
	ptc->P[0].x = 0;
	ptc->P[0].y = 0;
	ptc->P[0].z = 0;
	ptc->radius = (double *) malloc(sizeof(double) * 1);
	ptc->radius[0] = .5;
	update_bounds(ptc);
#endif

	return ptc;
}

void PtcFree(struct PointCloud *ptc)
{
	if (ptc == NULL)
		return;

	VecFree(ptc->P);
	free(ptc->radius);

	MEM_FREE(ptc);
}

void PtcAllocatePoint(struct PointCloud *ptc, int point_count)
{
	struct Vector *P_tmp = NULL;

	if (ptc == NULL)
		return;

	if (point_count < 1)
		return;

	P_tmp = VecRealloc(ptc->P, point_count);
	if (P_tmp == NULL)
		return;

	/* commit */
	ptc->P = P_tmp;
	ptc->point_count = point_count;

	{
		/* TODO TEST */
		int i;
		ptc->radius = MEM_ALLOC_ARRAY(double, point_count);
		for (i = 0; i < point_count; i++) {
			ptc->radius[i] = .02;
		}
	}
}

void PtcSetPosition(struct PointCloud *ptc, int index, const struct Vector *P)
{
	if (index < 0 && index >= ptc->point_count) {
		return;
	}
	ptc->P[index] = *P;
}

void PtcGetPosition(const struct PointCloud *ptc, int index, struct Vector *P)
{
	if (index < 0 && index >= ptc->point_count) {
		return;
	}
	*P = ptc->P[index];
}

void PtcComputeBounds(struct PointCloud *ptc)
{
	update_bounds(ptc);
}

void PtcGetPrimitiveSet(const struct PointCloud *ptc, struct PrimitiveSet *primset)
{
	MakePrimitiveSet(primset,
			"PointCloud",
			ptc,
			point_ray_intersect,
			point_bounds,
			point_cloud_bounds,
			point_count);
}

static int point_ray_intersect(const void *prim_set, int prim_id, double time,
		const struct Ray *ray, struct Intersection *isect)
{
/*
	X = o + t * d;
	(X - center) * (X - center) = R * R;
	|d|^2 * t^2 + 2 * d * (o - center) * t + |o - center|^2 - r^2 = 0;

	t = (-d * (o - center) +- sqrt(D)) / |d|^2;
	D = {d * (o - center)}^2 - |d|^2 * (|o - center|^2 - r^2);
*/
	const struct PointCloud *ptc = (const struct PointCloud *) prim_set;
	const struct Vector *center = &ptc->P[prim_id];
	const double radius = ptc->radius[prim_id];

	struct Vector orig_local = {0, 0, 0};
	double a = 0, b = 0, c = 0;
	double discriminant = 0, disc_sqrt;
	double t_hit = 0, t0 = 0, t1 = 0;

	orig_local.x = ray->orig.x - center->x;
	orig_local.y = ray->orig.y - center->y;
	orig_local.z = ray->orig.z - center->z;

	a = VEC3_DOT(&ray->dir, &ray->dir);
	b = VEC3_DOT(&ray->dir, &orig_local);
	c = VEC3_DOT(&orig_local, &orig_local) - radius * radius;

	discriminant = b * b - a * c;
	if (discriminant < 0)
		return 0;

	disc_sqrt = sqrt(discriminant);
	t0 = -b - disc_sqrt;
	t1 = -b + disc_sqrt;

	t_hit = MIN(t0, t1);
	if (t_hit < 0)
		return 0;

	POINT_ON_RAY(&isect->P, &ray->orig, &ray->dir, t_hit);
	isect->N.x = isect->P.x - center->x;
	isect->N.y = isect->P.y - center->y;
	isect->N.z = isect->P.z - center->z;
	VEC3_NORMALIZE(&isect->N);

	isect->object = NULL;
	isect->prim_id = prim_id;
	isect->t_hit = t_hit;

	return 1;
}

static void point_bounds(const void *prim_set, int prim_id, struct Box *bounds)
{
	const struct PointCloud *ptc = (const struct PointCloud *) prim_set;
	const struct Vector *P = &ptc->P[prim_id];
	const double radius = ptc->radius[prim_id];

	BOX3_SET(bounds,
			P->x - radius,
			P->y - radius,
			P->z - radius,
			P->x + radius,
			P->y + radius,
			P->z + radius);
}

static void point_cloud_bounds(const void *prim_set, struct Box *bounds)
{
	const struct PointCloud *ptc = (const struct PointCloud *) prim_set;
	*bounds = ptc->bounds;
}

static int point_count(const void *prim_set)
{
	const struct PointCloud *ptc = (const struct PointCloud *) prim_set;
	return ptc->point_count;
}

static void update_bounds(struct PointCloud *ptc)
{
	int i;

	BOX3_SET(&ptc->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX); 

	for (i = 0; i < ptc->point_count; i++) {
		const struct Vector *P = &ptc->P[i];
		const double radius = ptc->radius[i];
		struct Box ptbox;

		BOX3_SET(&ptbox,
				P->x - radius,
				P->y - radius,
				P->z - radius,
				P->x + radius,
				P->y + radius,
				P->z + radius);

		BoxAddBox(&ptc->bounds, &ptbox);
	}
}

