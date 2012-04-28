/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char *StrDup(const char *src);
extern char *StrFree(char *s);

extern char *StrCopyAndTerminate(char *dst, const char *src, size_t nchars);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

