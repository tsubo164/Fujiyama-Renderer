/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "PrimitiveSet.h"
#include "Box.h"
#include <stddef.h>
#include <float.h>

extern struct PrimitiveSet MakeInitialPrimitiveSet(void)
{
	struct PrimitiveSet p;

	p.primitive_name = "NullPrimitives";
	p.primitives = NULL;
	p.num_primitives = 0;
	BOX3_SET(p.bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	p.PrimitiveIntersect = NULL;
	p.PrimitiveBounds = NULL;

	return p;
}

extern struct PrimitiveSet MakePrimitiveSet(
		const char *primitive_name,
		const void *primitives, int num_primitives, const double *bounds,
		PrimIntersectFunction prim_intersect_function,
		PrimBoundsFunction prim_bounds_function)
{
	struct PrimitiveSet p;

	p.primitive_name = primitive_name;
	p.primitives = primitives;
	p.num_primitives = num_primitives;
	BOX3_COPY(p.bounds, bounds);

	p.PrimitiveIntersect = prim_intersect_function;
	p.PrimitiveBounds = prim_bounds_function;

	return p;
}

int PrimRayIntersect(const struct PrimitiveSet *prim_set, int prim_id,
		const struct Ray *ray, struct Intersection *isect)
{
	return prim_set->PrimitiveIntersect(prim_set->primitives, prim_id, ray, isect);
}

void PrimBounds(const struct PrimitiveSet *prim_set, int prim_id, double *bounds)
{
	prim_set->PrimitiveBounds(prim_set->primitives, prim_id, bounds);
}

