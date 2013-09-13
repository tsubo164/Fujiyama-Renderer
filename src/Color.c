/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Color.h"
#include "Memory.h"

struct Color *ColAlloc(long count)
{
  return SI_MEM_ALLOC_ARRAY(struct Color, count);
}

struct Color *ColRealloc(struct Color *c, long count)
{
  return SI_MEM_REALLOC_ARRAY(c, struct Color, count);
}

void ColFree(struct Color *c)
{
  SI_MEM_FREE(c);
}
