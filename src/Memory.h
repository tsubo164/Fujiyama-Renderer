/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef MEMORY_H
#define MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void *MemAlloc(size_t size);
void *MemRealloc(void *ptr, size_t size);
void MemFree(void *ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

