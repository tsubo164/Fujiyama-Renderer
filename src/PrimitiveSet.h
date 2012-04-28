/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef PRIMITIVESET_H
#define PRIMITIVESET_H

#ifdef __cplusplus
extern "C" {
#endif

struct Intersection;
struct Ray;

typedef int (*PrimIntersectFunction)(const void *prim_set, int prim_id, const struct Ray *ray,
			struct Intersection *isect);
typedef void (*PrimBoundsFunction)(const void *prim_set, int prim_id, double *bounds);

/* PrimitiveSet abstract a set of primitives that is used by Accelerator */
struct PrimitiveSet {
	const char *primitive_name;
	const void *primitives;
	int num_primitives;
	double bounds[6];

	PrimIntersectFunction PrimitiveIntersect;
	PrimBoundsFunction PrimitiveBounds;
};

/* Construction interfaces */
extern struct PrimitiveSet MakeInitialPrimitiveSet(void);
/* Each implementation of primitive set should make PrimitiveSet structure
   by caling this function */
extern struct PrimitiveSet MakePrimitiveSet(
		const char *primitive_name,
		const void *primitives, int num_primitives, const double *bounds,
		PrimIntersectFunction prim_intersect_function,
		PrimBoundsFunction prim_bounds_function);

/* interfaces called by Accelerator */
extern int PrimRayIntersect(const struct PrimitiveSet *prim_set, int prim_id,
		const struct Ray *ray, struct Intersection *isect);
extern void PrimBounds(const struct PrimitiveSet *prim_set, int prim_id, double *bounds);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

