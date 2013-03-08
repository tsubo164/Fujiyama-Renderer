/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Color.h"
#include <stdlib.h>

struct Color *ColAlloc(long count)
{
	return (struct Color *) malloc(sizeof(struct Color) * count);
}

struct Color *ColRealloc(struct Color *c, long count)
{
	return (struct Color *) realloc(c, sizeof(struct Color) * count);
}

void ColFree(struct Color *c)
{
	free(c);
}

