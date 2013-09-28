/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_tex_coord.h"
#include "fj_memory.h"

struct TexCoord *TexCoordAlloc(long count)
{
  return FJ_MEM_ALLOC_ARRAY(struct TexCoord, count);
}

struct TexCoord *TexCoordRealloc(struct TexCoord *texcoord, long count)
{
  return FJ_MEM_REALLOC_ARRAY(texcoord, struct TexCoord, count);
}

void TexCoordFree(struct TexCoord *texcoord)
{
  FJ_MEM_FREE(texcoord);
}

