/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Memory.h"
#include <stdlib.h>
#include <stdio.h>

void *MemAlloc(size_t size)
{
	void *ptr = malloc(size);

	if (ptr == NULL) {
		fprintf(stderr, "malloc in MemAlloc failed.\n");
		abort();
	}

	return ptr;
}

void *MemRealloc(void *ptr, size_t size)
{
	void *tmp = realloc(ptr, size);

	if (tmp == NULL) {
		fprintf(stderr, "realloc in MemRealloc failed.\n");
		abort();
	}

	return tmp;
}

void MemFree(void *ptr)
{
	free(ptr);
}

