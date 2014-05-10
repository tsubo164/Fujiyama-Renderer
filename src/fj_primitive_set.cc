/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_primitive_set.h"
#include "fj_box.h"
#include <stddef.h>
#include <float.h>

const static struct Box null_bounds;

int get_null_primitive_count(const void *primset)
{
  return 0;
}

void get_null_primitive_set_bounds(const void *primset, struct Box *bounds)
{
  *bounds = null_bounds;
}

int null_primitive_ray_intersect(const void *primset, int prim_id, double time,
    const struct Ray *ray, struct Intersection *isect)
{
  return 0;
}

void get_null_primitive_bounds(const void *primset, int prim_id, struct Box *bounds)
{
  *bounds = null_bounds;
}

void InitPrimitiveSet(struct PrimitiveSet *primset)
{
  MakePrimitiveSet(primset,
      "NullPrimitives",
      NULL,
      null_primitive_ray_intersect,
      get_null_primitive_bounds,
      get_null_primitive_set_bounds,
      get_null_primitive_count);
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

