/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_string_function.h"
#include "fj_numeric.h"
#include "fj_memory.h"
#include <string.h>

char *StrDup(const char *src)
{
  size_t alloc_size;
  char *dst;

  if (src == NULL)
    return NULL;

  alloc_size = strlen(src) + 1;
  dst = FJ_MEM_ALLOC_ARRAY(char, alloc_size);
  if (dst == NULL)
    return NULL;

  strncpy(dst, src, alloc_size);
  return dst;
}

char *StrFree(char *s)
{
  if (s != NULL)
    FJ_MEM_FREE(s);

  return NULL;
}

char *StrCopyAndTerminate(char *dst, const char *src, size_t nchars)
{
  size_t len;

  len = strlen(src);
  len = MIN(len, nchars);
  strncpy(dst, src, len);
  dst[len] = '\0';

  return dst;
}

