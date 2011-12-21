/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef ACCERALATOR_H
#define ACCERALATOR_H

#ifdef __cplusplus
extern "C" {
#endif

struct LocalGeometry;
struct Mesh;
struct Ray;

enum AcceleratorType {
	ACC_GRID = 0,
	ACC_BVH
};

typedef int (*PrimIntersectFunction)(const void *prim_set, int prim_id, const struct Ray *ray,
			struct LocalGeometry *isect, double *t_hit);
typedef void (*PrimBoundsFunction)(const void *prim_set, int prim_id, double *bounds);

extern struct Accelerator *AccNew(int accelerator_type);
extern void AccFree(struct Accelerator *acc);

extern int AccBuild(struct Accelerator *acc);
extern void AccGetBounds(const struct Accelerator *acc, double *bounds);
extern void AccSetTargetGeometry(struct Accelerator *acc,
	const void *primset, int nprims, const double *primset_bounds,
	PrimIntersectFunction prim_intersect_function,
	PrimBoundsFunction prim_bounds_function);

extern int AccIntersect(const struct Accelerator *acc, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

