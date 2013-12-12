/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_ARRAY_H
#define FJ_ARRAY_H

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

/* request change in capacity, then returns the head of data. */
extern char *ArrReserve(struct Array *a, size_t new_alloc);

/* request change in number of elements, then returns the head of data. */
extern char *ArrResize(struct Array *a, size_t new_size);

extern char *ArrGet(const struct Array *a, int index);

extern size_t ArrGetElementCount(const struct Array *a);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
