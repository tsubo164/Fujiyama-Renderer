/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef IO_H
#define IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

extern int IOMatchMagic(FILE *file, const char *magic);
extern int IOMatchVersion(FILE *file, int version);

extern size_t IOReadString(FILE *file, char *dst, size_t dstsize);
extern size_t IOReadInt(FILE *file, int *dst, size_t nelems);
extern size_t IOReadFloat(FILE *file, float *dst, size_t nelems);
extern size_t IOReadDouble(FILE *file, double *dst, size_t nelems);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

