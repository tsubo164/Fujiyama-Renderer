/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_IFF_H
#define FJ_IFF_H

#include "fj_compatibility.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t DataSize;

/* This is an iff-inspired file format module.
 * EA IFF 85: Standard for Interchange Format Files */
typedef struct IffFile IffFile;

/* Open/Close iff file */
extern IffFile *IffOpen(const char *filename, const char *modes);
extern void IffClose(IffFile *iff);

/* Read/Write functions for primitive types */
extern size_t IffWriteInt8(IffFile *iff, const int8_t *data, size_t count);
extern size_t IffWriteInt16(IffFile *iff, const int16_t *data, size_t count);
extern size_t IffWriteInt32(IffFile *iff, const int32_t *data, size_t count);
extern size_t IffWriteInt64(IffFile *iff, const int64_t *data, size_t count);
extern size_t IffWriteString(IffFile *iff, const char *data);

extern size_t IffReadInt8(IffFile *iff, int8_t *data, size_t count);
extern size_t IffReadInt16(IffFile *iff, int16_t *data, size_t count);
extern size_t IffReadInt32(IffFile *iff, int32_t *data, size_t count);
extern size_t IffReadInt64(IffFile *iff, int64_t *data, size_t count);
extern size_t IffReadString(IffFile *iff, char *data);

/* Write functions for primitive types with chunk id */
extern size_t IffWriteChunkInt8(IffFile *iff, const char *chunk_id,
    const int8_t *data, size_t count);
extern size_t IffWriteChunkInt16(IffFile *iff, const char *chunk_id,
    const int16_t *data, size_t count);
extern size_t IffWriteChunkInt32(IffFile *iff, const char *chunk_id,
    const int32_t *data, size_t count);
extern size_t IffWriteChunkInt64(IffFile *iff, const char *chunk_id,
    const int64_t *data, size_t count);

#define CHUNK_ID_SIZE 8
typedef struct IffChunk {
  char id[CHUNK_ID_SIZE];
  DataSize data_head;
  DataSize data_size;
} IffChunk;

extern void IffWriteChunkGroupBegin(IffFile *iff, const char *chunk_id, DataSize *begin_pos);
extern void IffWriteChunkGroupEnd(IffFile *iff, size_t begin_pos);

extern void IffReadChunkGroupBegin(IffFile *iff, IffChunk *group_chunk);
extern void IffReadChunkGroupEnd(IffFile *iff, IffChunk *group_chunk);

extern int IffReadNextChunk(IffFile *iff, IffChunk *chunk);
extern void IffSkipCurrentChunk(IffFile *iff, const IffChunk *chunk);
extern int IffChunkMatch(const IffChunk *chunk, const char *key);
extern int IffEndOfChunk(const IffFile *iff, const IffChunk *chunk);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
