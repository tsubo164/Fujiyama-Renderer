/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "TexCoord.h"
#include "Memory.h"

struct TexCoord *TexCoordAlloc(long count)
{
	return MEM_ALLOC_ARRAY(struct TexCoord, count);
}

struct TexCoord *TexCoordRealloc(struct TexCoord *texcoord, long count)
{
	return MEM_REALLOC_ARRAY(texcoord, struct TexCoord, count);
}

void TexCoordFree(struct TexCoord *texcoord)
{
	MEM_FREE(texcoord);
}

