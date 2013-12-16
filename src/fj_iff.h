/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_IFF_H
#define FJ_IFF_H

#include "fj_compatibility.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t DataSize;

/* This is an iff-inspired file format module.
 * EA IFF 85: Standard for Interchange Format Files */
typedef struct IffFile IffFile;

/* Open/Close iff file */
extern IffFile *IffOpen(const char *filename, const char *modes);
extern void IffClose(IffFile *iff);

/* TODO test generic interface */
extern DataSize IffWrite(IffFile *iff, const void *data, DataSize size, DataSize count);
#define FJ_IFF_WRITE(iff,data,count) IffWrite(iff, data, sizeof(*(data)), count)
extern DataSize IffWriteChunk(IffFile *iff, const char *chunk_id,
    const void *data, DataSize size, DataSize count);
#define FJ_IFF_WRITE_CHUNK(iff,chunk_id,data,count) IffWriteChunk(iff, chunk_id, data, sizeof(*(data)), count)

/* Read/Write functions for primitive types */
extern DataSize IffWriteInt8(IffFile *iff, const int8_t *data, DataSize count);
extern DataSize IffWriteInt16(IffFile *iff, const int16_t *data, DataSize count);
extern DataSize IffWriteInt32(IffFile *iff, const int32_t *data, DataSize count);
extern DataSize IffWriteInt64(IffFile *iff, const int64_t *data, DataSize count);
extern DataSize IffWriteFloat(IffFile *iff, const float *data, DataSize count);
extern DataSize IffWriteDouble(IffFile *iff, const double *data, DataSize count);
extern DataSize IffWriteString(IffFile *iff, const char *data);

extern DataSize IffReadInt8(IffFile *iff, int8_t *data, DataSize count);
extern DataSize IffReadInt16(IffFile *iff, int16_t *data, DataSize count);
extern DataSize IffReadInt32(IffFile *iff, int32_t *data, DataSize count);
extern DataSize IffReadInt64(IffFile *iff, int64_t *data, DataSize count);
extern DataSize IffReadFloat(IffFile *iff, float *data, DataSize count);
extern DataSize IffReadDouble(IffFile *iff, double *data, DataSize count);
extern DataSize IffReadString(IffFile *iff, char *data);

/* Write functions for primitive types with chunk id */
extern DataSize IffWriteChunkInt8(IffFile *iff, const char *chunk_id,
    const int8_t *data, DataSize count);
extern DataSize IffWriteChunkInt16(IffFile *iff, const char *chunk_id,
    const int16_t *data, DataSize count);
extern DataSize IffWriteChunkInt32(IffFile *iff, const char *chunk_id,
    const int32_t *data, DataSize count);
extern DataSize IffWriteChunkInt64(IffFile *iff, const char *chunk_id,
    const int64_t *data, DataSize count);
extern DataSize IffWriteChunkFloat(IffFile *iff, const char *chunk_id,
    const float *data, DataSize count);
extern DataSize IffWriteChunkDouble(IffFile *iff, const char *chunk_id,
    const double *data, DataSize count);
extern DataSize IffWriteChunkString(IffFile *iff, const char *chunk_id,
    const char *data);

enum { CHUNK_ID_SIZE = 8 };
typedef struct IffChunk {
  char id[CHUNK_ID_SIZE];
  DataSize data_head;
  DataSize data_size;
} IffChunk;

extern void IffWriteChunkGroupBegin(IffFile *iff, const char *chunk_id, IffChunk *group_chunk);
extern void IffWriteChunkGroupEnd(IffFile *iff, IffChunk *group_chunk);

extern int IffReadChunkGroupBegin(IffFile *iff, const char *chunk_id, IffChunk *group_chunk);
extern void IffReadChunkGroupEnd(IffFile *iff, IffChunk *group_chunk);

extern int IffReadNextChunk(IffFile *iff, IffChunk *chunk);
extern void IffPutBackChunk(IffFile *iff, const IffChunk *chunk);
extern void IffSkipCurrentChunk(IffFile *iff, const IffChunk *chunk);

extern int IffChunkMatch(const IffChunk *chunk, const char *key);
extern int IffEndOfChunk(const IffFile *iff, const IffChunk *chunk);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
