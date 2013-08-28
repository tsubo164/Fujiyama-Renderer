/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Vector.h"
#include "Memory.h"
#include <stdio.h>

void VecPrint(const struct Vector *a)
{
  printf("(%g, %g, %g)\n", a->x, a->y, a->z);
}

struct Vector *VecAlloc(long count)
{
  return MEM_ALLOC_ARRAY(struct Vector, count);
}

struct Vector *VecRealloc(struct Vector *v, long count)
{
  return MEM_REALLOC_ARRAY(v, struct Vector, count);
}

void VecFree(struct Vector *v)
{
  MEM_FREE(v);
}
