/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "PrimitiveSet.h"
#include <stddef.h>
#include <float.h>

void InitPrimitiveSet(struct PrimitiveSet *primset)
{
	primset->name = "NullPrimitives";
	primset->data = NULL;

	primset->PrimitiveIntersect = NULL;
	primset->PrimitiveBounds = NULL;
	primset->PrimitiveSetBounds = NULL;
	primset->PrimitiveCount = NULL;
}

void MakePrimitiveSet(struct PrimitiveSet *primset,
		const char *primset_name,
		const void *primset_data,
		PrimIntersectFunction prim_intersect_function,
		PrimBoundsFunction prim_bounds_function,
		PrimSetBoundsFunction primset_bounds_function,
		PrimCountFunction prim_count_function)
{
	primset->name = primset_name;
	primset->data = primset_data;

	primset->PrimitiveIntersect = prim_intersect_function;
	primset->PrimitiveBounds = prim_bounds_function;
	primset->PrimitiveSetBounds = primset_bounds_function;
	primset->PrimitiveCount = prim_count_function;
}

const char *PrmGetName(const struct PrimitiveSet *primset)
{
	return primset->name;
}

int PrmGetPrimitiveCount(const struct PrimitiveSet *primset)
{
	return primset->PrimitiveCount(primset->data);
}

void PrmGetBounds(const struct PrimitiveSet *primset, struct Box *bounds)
{
	primset->PrimitiveSetBounds(primset->data, bounds);
}

int PrmRayIntersect(const struct PrimitiveSet *primset, int prim_id, double time,
		const struct Ray *ray, struct Intersection *isect)
{
	return primset->PrimitiveIntersect(primset->data, prim_id, time, ray, isect);
}

void PrmGetPrimitiveBounds(const struct PrimitiveSet *primset,
		int prim_id, struct Box *bounds)
{
	primset->PrimitiveBounds(primset->data, prim_id, bounds);
}

