/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef PRIMITIVESET_H
#define PRIMITIVESET_H

#ifdef __cplusplus
extern "C" {
#endif

struct Intersection;
struct Ray;

typedef int (*PrimIntersectFunction)(const void *primset, int prim_id, double time,
			const struct Ray *ray, struct Intersection *isect);
typedef void (*PrimBoundsFunction)(const void *primset, int prim_id, double *bounds);

/* PrimitiveSet abstract a set of primitives that is used by Accelerator */
struct PrimitiveSet {
	const char *name;
	const void *data;
	int nprims;
	double bounds[6];

	PrimIntersectFunction PrimitiveIntersect;
	PrimBoundsFunction PrimitiveBounds;
};

/* Each primitive set should make PrimitiveSet by caling this function */
extern void MakePrimitiveSet(struct PrimitiveSet *primset,
		const char *primset_name,
		const void *primset_data, int nprims, const double *bounds,
		PrimIntersectFunction prim_intersect_function,
		PrimBoundsFunction prim_bounds_function);
extern void InitPrimitiveSet(struct PrimitiveSet *primset);

extern const char *PrmGetName(const struct PrimitiveSet *primset);
extern int PrmGetPrimitiveCount(const struct PrimitiveSet *primset);
extern void PrmGetBounds(const struct PrimitiveSet *primset, double *bounds);
extern int PrmRayIntersect(const struct PrimitiveSet *primset, int prim_id, double time,
		const struct Ray *ray, struct Intersection *isect);
extern void PrmGetPrimitiveBounds(const struct PrimitiveSet *primset, int prim_id, double *bounds);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

