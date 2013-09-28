/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_tex_coord.h"
#include "fj_memory.h"

struct TexCoord *TexCoordAlloc(long count)
{
  return SI_MEM_ALLOC_ARRAY(struct TexCoord, count);
}

struct TexCoord *TexCoordRealloc(struct TexCoord *texcoord, long count)
{
  return SI_MEM_REALLOC_ARRAY(texcoord, struct TexCoord, count);
}

void TexCoordFree(struct TexCoord *texcoord)
{
  SI_MEM_FREE(texcoord);
}

