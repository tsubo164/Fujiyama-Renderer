/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_STRINGFUNCTION_H
#define FJ_STRINGFUNCTION_H

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

#endif /* FJ_XXX_H */
