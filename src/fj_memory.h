/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_MEMORY_H
#define FJ_MEMORY_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* force type to avoid missing sizeof mulptiplication */
#define FJ_MEM_ALLOC_ARRAY(type,nelems) ((type*) malloc(sizeof(type) * (nelems)))

/* for single object */
#define FJ_MEM_ALLOC(type) (FJ_MEM_ALLOC_ARRAY(type, 1))

#define FJ_MEM_REALLOC_ARRAY(ptr,type,nelems) ((type*) realloc((ptr), sizeof(type) * (nelems)))

#define FJ_MEM_FREE(ptr) (free((ptr)))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
