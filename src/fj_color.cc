/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_color.h"
#include "fj_memory.h"

namespace fj {

struct Color *ColAlloc(long count)
{
  return FJ_MEM_ALLOC_ARRAY(struct Color, count);
}

struct Color *ColRealloc(struct Color *c, long count)
{
  return FJ_MEM_REALLOC_ARRAY(c, struct Color, count);
}

void ColFree(struct Color *c)
{
  FJ_MEM_FREE(c);
}

} //  namespace xxx
