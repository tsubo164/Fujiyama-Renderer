/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "String.h"
#include "Numeric.h"
#include <stdlib.h>
#include <string.h>

char *StrDup(const char *src)
{
	size_t alloc_size;
	char *dst;

	if (src == NULL)
		return NULL;

	alloc_size = strlen(src) + 1;
	dst = (char *) malloc(sizeof(char) * alloc_size);
	if (dst == NULL)
		return NULL;

	strncpy(dst, src, alloc_size);
	return dst;
}

char *StrFree(char *s)
{
	if (s != NULL)
		free(s);

	return NULL;
}

char *StrCopyMax(char *dst, const char *src, size_t maxsize)
{
	size_t ncpys;

	ncpys = strlen(src) + 1;
	ncpys = MIN(ncpys, maxsize);
	strncpy(dst, src, ncpys);

	return dst;
}

