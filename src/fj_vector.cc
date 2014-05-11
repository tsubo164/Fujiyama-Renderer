/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_vector.h"
#include "fj_memory.h"
#include <stdio.h>

namespace fj {

void VecPrint(const struct Vector *a)
{
  printf("(%g, %g, %g)\n", a->x, a->y, a->z);
}

struct Vector *VecAlloc(long count)
{
  return FJ_MEM_ALLOC_ARRAY(struct Vector, count);
}

struct Vector *VecRealloc(struct Vector *v, long count)
{
  return FJ_MEM_REALLOC_ARRAY(v, struct Vector, count);
}

void VecFree(struct Vector *v)
{
  FJ_MEM_FREE(v);
}

} // namespace xxx
