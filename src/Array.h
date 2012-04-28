/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Array {
	size_t elemsize;
	size_t nelems;
	size_t nallocs;
	char *data;
};

extern struct Array *ArrNew(size_t size_of_element);
extern void ArrFree(struct Array *a);

/* returns the pointer of the head of array */
extern char *ArrPush(struct Array *a, const void *data);

/* returns the pointer of the head of array */
extern char *ArrPushPointer(struct Array *a, const void *pointer);

/* grows array if needed, then returns the head of data. */
extern char *ArrGrow(struct Array *a, size_t new_alloc);

extern char *ArrGet(const struct Array *a, int index);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

