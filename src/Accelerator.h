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

/* data structure and functions for derived */
struct DerivedAccelerator {
	char *self_ptr;
};

typedef void *(*NewDerivedFunction)(void);
typedef void (*FreeDerivedFunction)(void *derived);
typedef int (*BuildDerivedFunction)(void *derived, const struct PrimitiveSet *primset);
typedef int (*IntersectDerivedFunction)(void *derived, const struct PrimitiveSet *primset,
			double time, const struct Ray *ray, struct Intersection *isect);
typedef const char *(*GetDerivedNameFunction)(void);

extern void AccSetDerivedFunctions(struct Accelerator *acc,
		NewDerivedFunction       new_derived_function,
		FreeDerivedFunction      free_derived_function,
		BuildDerivedFunction     build_derived_function,
		IntersectDerivedFunction intersect_derived_function,
		GetDerivedNameFunction   get_derived_name_function);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

