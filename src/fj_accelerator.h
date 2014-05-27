/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_ACCERALATOR_H
#define FJ_ACCERALATOR_H

namespace fj {

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

extern double AccGetBoundsPadding(void);
extern void AccGetBounds(const struct Accelerator *acc, struct Box *bounds);
extern void AccSetPrimitiveSet(struct Accelerator *acc, struct PrimitiveSet *primset);

extern void AccComputeBounds(struct Accelerator *acc);
extern int AccBuild(struct Accelerator *acc);
extern int AccIntersect(const struct Accelerator *acc, double time,
    const struct Ray *ray, struct Intersection *isect);

/* data structure and functions for derived accelerators */
typedef char * DerivedAccelerator;

typedef DerivedAccelerator (*NewDerivedFunction)(void);
typedef void (*FreeDerivedFunction)(DerivedAccelerator derived);
typedef int (*BuildDerivedFunction)(DerivedAccelerator derived,
    const struct PrimitiveSet *primset);
typedef int (*IntersectDerivedFunction)(DerivedAccelerator derived,
    const struct PrimitiveSet *primset, double time, const struct Ray *ray,
    struct Intersection *isect);
typedef const char *(*GetDerivedNameFunction)(void);

extern void AccSetDerivedFunctions(struct Accelerator *acc,
    NewDerivedFunction       new_derived_function,
    FreeDerivedFunction      free_derived_function,
    BuildDerivedFunction     build_derived_function,
    IntersectDerivedFunction intersect_derived_function,
    GetDerivedNameFunction   get_derived_name_function);

} // namespace xxx

#endif /* FJ_XXX_H */
