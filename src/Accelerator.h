/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef ACCERALATOR_H
#define ACCERALATOR_H

#ifdef __cplusplus
extern "C" {
#endif

struct Accelerator;
struct Intersection;
struct PrimitiveSet;
struct Ray;

enum AcceleratorType {
	ACC_GRID = 0,
	ACC_BVH
};

extern struct Accelerator *AccNew(int accelerator_type);
extern void AccFree(struct Accelerator *acc);

extern void AccGetBounds(const struct Accelerator *acc, double *bounds);
extern void AccSetPrimitiveSet(struct Accelerator *acc, const struct PrimitiveSet *primset);

extern int AccBuild(struct Accelerator *acc);
extern int AccIntersect(const struct Accelerator *acc, const struct Ray *ray,
		struct Intersection *isect);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

