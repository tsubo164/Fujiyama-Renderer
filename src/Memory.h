/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* force type to avoid missing sizeof mulptiplication */
#define SI_MEM_ALLOC_ARRAY(type,nelems) ((type*) malloc(sizeof(type) * (nelems)))

/* for single object */
#define SI_MEM_ALLOC(type) (SI_MEM_ALLOC_ARRAY(type, 1))

#define SI_MEM_REALLOC_ARRAY(ptr,type,nelems) ((type*) realloc((ptr), sizeof(type) * (nelems)))

#define SI_MEM_FREE(ptr) (free((ptr)))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */
