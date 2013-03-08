/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef ACCERALATOR_H
#define ACCERALATOR_H

#ifdef __cplusplus
extern "C" {
#endif

enum AcceleratorType {
	ACC_GRID = 0,
	ACC_BVH
};

struct Accelerator;
struct Intersection;
struct PrimitiveSet;
struct Box;
struct Ray;

extern struct Accelerator *AccNew(int accelerator_type);
extern void AccFree(struct Accelerator *acc);

extern void AccGetBounds(const struct Accelerator *acc, struct Box *bounds);
extern void AccSetPrimitiveSet(struct Accelerator *acc, const struct PrimitiveSet *primset);

extern void AccComputeBounds(struct Accelerator *acc);
extern int AccBuild(struct Accelerator *acc);
extern int AccIntersect(const struct Accelerator *acc, double time,
		const struct Ray *ray, struct Intersection *isect);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

