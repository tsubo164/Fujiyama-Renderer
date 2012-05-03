/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "PrimitiveSet.h"
#include "Box.h"
#include <stddef.h>
#include <float.h>

void InitPrimitiveSet(struct PrimitiveSet *primset)
{
	primset->name = "NullPrimitives";
	primset->data = NULL;
	primset->nprims = 0;
	BOX3_SET(primset->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	primset->PrimitiveIntersect = NULL;
	primset->PrimitiveBounds = NULL;
}

void MakePrimitiveSet(struct PrimitiveSet *primset,
		const char *primset_name,
		const void *primset_data, int nprims, const double *bounds,
		PrimIntersectFunction prim_intersect_function,
		PrimBoundsFunction prim_bounds_function)
{
	primset->name = primset_name;
	primset->data = primset_data;
	primset->nprims = nprims;
	BOX3_COPY(primset->bounds, bounds);

	primset->PrimitiveIntersect = prim_intersect_function;
	primset->PrimitiveBounds = prim_bounds_function;
}

const char *PrmGetName(const struct PrimitiveSet *primset)
{
	return primset->name;
}

int PrmGetPrimitiveCount(const struct PrimitiveSet *primset)
{
	return primset->nprims;
}

void PrmGetBounds(const struct PrimitiveSet *primset, double *bounds)
{
	BOX3_COPY(bounds, primset->bounds);
}

int PrmRayIntersect(const struct PrimitiveSet *primset, int prim_id,
		const struct Ray *ray, struct Intersection *isect)
{
	return primset->PrimitiveIntersect(primset->data, prim_id, ray, isect);
}

void PrmGetPrimitiveBounds(const struct PrimitiveSet *primset, int prim_id, double *bounds)
{
	primset->PrimitiveBounds(primset->data, prim_id, bounds);
}

