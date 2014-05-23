/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_PRIMITIVESET_H
#define FJ_PRIMITIVESET_H

#include "fj_types.h"

namespace fj {

struct Intersection;
struct Box;
struct Ray;

typedef int (*PrimIntersectFunction)(const void *primset, int prim_id, double time,
      const struct Ray *ray, struct Intersection *isect);
typedef void (*PrimBoundsFunction)(const void *primset, int prim_id, struct Box *bounds);
typedef void (*PrimSetBoundsFunction)(const void *primset, struct Box *bounds);
typedef int (*PrimCountFunction)(const void *primset);

/* PrimitiveSet abstract a set of primitives that is used by Accelerator */
struct PrimitiveSet {
  const char *name;
  const void *data;

  PrimIntersectFunction PrimitiveIntersect;
  PrimBoundsFunction PrimitiveBounds;
  PrimSetBoundsFunction PrimitiveSetBounds;
  PrimCountFunction PrimitiveCount;

  PrimitiveSet() :
      PrimitiveIntersect(0),
      PrimitiveBounds(0),
      PrimitiveSetBounds(0),
      PrimitiveCount(0)
  {
  }
  virtual ~PrimitiveSet() {}

  bool RayIntersect(Index prim_id, Real time, const Ray &ray, Intersection *isect) const
  {
    return ray_intersect(prim_id, time, ray, isect);
  }
  void GetPrimitiveBounds(Index prim_id, Box *bounds) const
  {
    get_primitive_bounds(prim_id, bounds);
  }
  void GetBounds(Box *bounds) const
  {
    get_bounds(bounds);
  }
  Index GetPrimitiveCount() const
  {
    return get_primitive_count();
  }

public:
  virtual bool ray_intersect(Index prim_id, Real time,
      const Ray &ray, Intersection *isect) const
  { return false; }
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const
  {}
  virtual void get_bounds(Box *bounds) const
  {}
  virtual Index get_primitive_count() const
  { return 0; }
};

/* Each primitive set should make PrimitiveSet by caling this function */
extern void MakePrimitiveSet(struct PrimitiveSet *primset,
    const char *primset_name,
    const void *primset_data,
    PrimIntersectFunction prim_intersect_function,
    PrimBoundsFunction prim_bounds_function,
    PrimSetBoundsFunction primset_bounds_function,
    PrimCountFunction prim_count_function);

extern void InitPrimitiveSet(struct PrimitiveSet *primset);

extern const char *PrmGetName(const struct PrimitiveSet *primset);
extern int PrmGetPrimitiveCount(const struct PrimitiveSet *primset);
extern void PrmGetBounds(const struct PrimitiveSet *primset, struct Box *bounds);
extern int PrmRayIntersect(const struct PrimitiveSet *primset, int prim_id, double time,
    const struct Ray *ray, struct Intersection *isect);
extern void PrmGetPrimitiveBounds(const struct PrimitiveSet *primset,
    int prim_id, struct Box *bounds);

} // namespace xxx

#endif /* FJ_XXX_H */
