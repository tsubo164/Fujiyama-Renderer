/*
Copyright (c) 2011-2016 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_file_io.h"

namespace fj {

#define DEFINE_FILE_READ_WRITE(SUFFIX,TYPE) \
int FjFile_Read##SUFFIX(FILE *file, TYPE *dst, size_t count) \
{ \
  const size_t nreads = fread(dst, sizeof(*dst), count, file); \
  if (nreads != count) \
    return -1; \
  else \
    return 0; \
} \
int FjFile_Write##SUFFIX(FILE *file, const TYPE *src, size_t count) \
{ \
  const size_t nwrotes = fwrite(src, sizeof(*src), count, file); \
  if (nwrotes != count) \
    return -1; \
  else \
    return 0; \
}

DEFINE_FILE_READ_WRITE(Int8,   int8_t)
DEFINE_FILE_READ_WRITE(Int32,  int32_t)
DEFINE_FILE_READ_WRITE(Int64,  int64_t)
DEFINE_FILE_READ_WRITE(Float,  float)
DEFINE_FILE_READ_WRITE(Double, double)

} // namespace xxx
