/*
Copyright (c) 2011-2015 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_FJ_FILE_IO_H
#define FJ_FJ_FILE_IO_H

#include "fj_compatibility.h"
#include <stdio.h>

namespace fj {

int FjFile_ReadInt8    (FILE *file, int8_t  *dst, size_t count);
int FjFile_ReadInt32   (FILE *file, int32_t *dst, size_t count);
int FjFile_ReadInt64   (FILE *file, int64_t *dst, size_t count);
int FjFile_ReadFloat   (FILE *file, float   *dst, size_t count);
int FjFile_ReadDouble  (FILE *file, double  *dst, size_t count);

int FjFile_WriteInt8   (FILE *file, const int8_t  *src, size_t count);
int FjFile_WriteInt32  (FILE *file, const int32_t *src, size_t count);
int FjFile_WriteInt64  (FILE *file, const int64_t *src, size_t count);
int FjFile_WriteFloat  (FILE *file, const float   *src, size_t count);
int FjFile_WriteDouble (FILE *file, const double  *src, size_t count);

} // namespace xxx

#endif /* FJ_XXX_H */
