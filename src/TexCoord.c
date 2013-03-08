/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "TexCoord.h"
#include <stdlib.h>

struct TexCoord *TexCoordAlloc(long count)
{
	return (struct TexCoord *) malloc(sizeof(struct TexCoord) * count);
}

struct TexCoord *TexCoordRealloc(struct TexCoord *texcoord, long count)
{
	return (struct TexCoord *) realloc(texcoord, sizeof(struct TexCoord) * count);
}

void TexCoordFree(struct TexCoord *texcoord)
{
	free(texcoord);
}

