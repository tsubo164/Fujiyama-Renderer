/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Array.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct Array *ArrNew(size_t size_of_element)
{
	struct Array *a;
	assert(size_of_element > 0);

	a = (struct Array *) malloc(sizeof(struct Array));
	if (a == NULL)
		return NULL;

	a->elemsize = size_of_element;
	a->nelems = 0;
	a->nallocs = 0;
	a->data = NULL;

	return a;
}

void ArrFree(struct Array *a)
{
	if (a == NULL)
		return;

	if (a->nallocs > 0)
		free(a->data);

	free(a);
}

char *ArrPush(struct Array *a, const void *data)
{
	char *dst;
	const char *src;
	assert(data != NULL);

	if (a->nelems == a->nallocs) {
		const size_t new_alloc = a->nallocs == 0 ? 1 : 2 * a->nallocs;
		ArrGrow(a, new_alloc);
	}

	dst = a->data + a->elemsize * a->nelems;
	src = data;
	memcpy(dst, src, a->elemsize);
	a->nelems++;

	return a->data;
}

char *ArrPushPointer(struct Array *a, const void *pointer)
{
	const void *p = pointer;
	return ArrPush(a, &p);
}

char *ArrGrow(struct Array *a, size_t new_alloc)
{
	if (a->nallocs < new_alloc) {
		const size_t new_size = a->elemsize * new_alloc;
		a->data = (char *) realloc(a->data, new_size);
		a->nallocs = new_alloc;
	}
	return a->data;
}

char *ArrGet(const struct Array *a, int index)
{
	return a->data + index * a->elemsize;
}

