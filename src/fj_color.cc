/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_color.h"
#include "fj_memory.h"

namespace fj {

Color *ColAlloc(long count)
{
  return FJ_MEM_ALLOC_ARRAY(Color, count);
}

Color *ColRealloc(Color *c, long count)
{
  return FJ_MEM_REALLOC_ARRAY(c, Color, count);
}

void ColFree(Color *c)
{
  FJ_MEM_FREE(c);
}

} //  namespace xxx
