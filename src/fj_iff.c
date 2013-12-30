/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_iff.h"
#include "fj_memory.h"
#include <stdio.h>
#include <string.h>

struct IffFile {
  FILE *file;
};

IffFile *IffOpen(const char *filename, const char *modes)
{
  IffFile *iff = FJ_MEM_ALLOC(IffFile);
  if (iff == NULL) {
    return NULL;
  }

  iff->file = fopen(filename, modes);
  if (iff->file == NULL) {
    IffClose(iff);
    return NULL;
  }

  return iff;
}

void IffClose(IffFile *iff)
{
  if (iff == NULL) {
    return;
  }

  fclose(iff->file);
  FJ_MEM_FREE(iff);
}

static DataSize iff_read(const IffFile *iff, void *data, DataSize size, DataSize count)
{
  return fread(data, size, count, iff->file);
}

static DataSize iff_write(IffFile *iff, const void *data, DataSize size, DataSize count)
{
  return fwrite(data, size, count, iff->file);
}

static DataSize iff_tell(const IffFile *iff)
{
  return ftell(iff->file);
}

enum {
  IFF_SEEK_SET = SEEK_SET,
  IFF_SEEK_CUR = SEEK_CUR,
  IFF_SEEK_END = SEEK_END
};
static int iff_seek(IffFile *iff, DataSize offset, int base)
{
  return fseek(iff->file, offset, base);
}

#define DEFINE_READ_WRITE_(SUFFIX,TYPE) \
DataSize IffWrite##SUFFIX(IffFile *iff, const TYPE *data, DataSize count) \
{ \
  const DataSize nwrotes = iff_write(iff, data, sizeof(*data), count); \
  return nwrotes * sizeof(*data); \
} \
DataSize IffRead##SUFFIX(IffFile *iff, TYPE *data, DataSize count) \
{ \
  const DataSize nreads = fread(data, sizeof(*data), count, iff->file); \
  return nreads * sizeof(*data); \
}
DEFINE_READ_WRITE_(Int8, int8_t)
DEFINE_READ_WRITE_(Int16, int16_t)
DEFINE_READ_WRITE_(Int32, int32_t)
DEFINE_READ_WRITE_(Int64, int64_t)
DEFINE_READ_WRITE_(Float, float)
DEFINE_READ_WRITE_(Double, double)

DataSize IffReadString(IffFile *iff, char *data)
{
  DataSize data_size = 0;
  const DataSize start = iff_tell(iff);
  iff_read(iff, &data_size, sizeof(data_size), 1);
  iff_read(iff, data,       sizeof(*data),     data_size);
  return iff_tell(iff) - start;
}

DataSize IffWriteString(IffFile *iff, const char *data)
{
  const DataSize data_size = sizeof(*data) * (strlen(data) + 1);
  const DataSize start = iff_tell(iff);
  iff_write(iff, &data_size, sizeof(data_size), 1);
  iff_write(iff, data,       data_size,         1);
  return iff_tell(iff) - start;
}

static void write_chunk_id(IffFile *iff, const char *chunk_id)
{
  char id[CHUNK_ID_SIZE] = {'\0'};
  strncpy(id, chunk_id, CHUNK_ID_SIZE);
  iff_write(iff, id, CHUNK_ID_SIZE, 1);
}

static void write_padding(IffFile *iff)
{
  const DataSize cur = iff_tell(iff);
  if (cur % 2 == 1) {
    int8_t c = '\0';
    iff_write(iff, &c, sizeof(c), 1);
  }
}

static void read_padding(IffFile *iff)
{
  const DataSize cur = iff_tell(iff);
  if (cur % 2 == 1) {
    int8_t c = '\0';
    iff_read(iff, &c, sizeof(c), 1);
  }
}

#define DEFINE_WRITE_CHUNK_(SUFFIX,TYPE) \
DataSize IffWriteChunk##SUFFIX(IffFile *iff, const char *chunk_id, \
    const TYPE *data, DataSize count) \
{ \
  const DataSize data_size = count * sizeof(*data); \
  DataSize start = 0; \
  write_padding(iff); \
  start = iff_tell(iff); \
  write_chunk_id(iff, chunk_id); \
  iff_write(iff, &data_size, sizeof(data_size), 1); \
  iff_write(iff, data,       data_size,         1); \
  return iff_tell(iff) - start; \
}

DEFINE_WRITE_CHUNK_(Int8, int8_t)
DEFINE_WRITE_CHUNK_(Int16, int16_t)
DEFINE_WRITE_CHUNK_(Int32, int32_t)
DEFINE_WRITE_CHUNK_(Int64, int64_t)
DEFINE_WRITE_CHUNK_(Float, float)
DEFINE_WRITE_CHUNK_(Double, double)

DataSize IffWriteChunkString(IffFile *iff, const char *chunk_id, const char *data)
{
  DataSize start = 0;
  IffChunk chunk;

  write_padding(iff);
  start = iff_tell(iff);

  IffWriteChunkGroupBegin(iff, chunk_id, &chunk);
  IffWriteString(iff, data);
  IffWriteChunkGroupEnd(iff, &chunk);

  return iff_tell(iff) - start;
}

void IffWriteChunkGroupBegin(IffFile *iff, const char *chunk_id, IffChunk *group_chunk)
{
  const DataSize default_data_size = 0;

  write_padding(iff);
  write_chunk_id(iff, chunk_id);
  iff_write(iff, &default_data_size, sizeof(default_data_size), 1);
  group_chunk->data_head = iff_tell(iff);
}

void IffWriteChunkGroupEnd(IffFile *iff, IffChunk *group_chunk)
{
  const DataSize bytes = iff_tell(iff) - group_chunk->data_head;

  if (bytes == 0) {
    return;
  }

  iff_seek(iff, -bytes - sizeof(DataSize), IFF_SEEK_CUR);
  iff_write(iff, &bytes, sizeof(bytes), 1);
  iff_seek(iff, bytes, IFF_SEEK_CUR);
}

int IffReadChunkGroupBegin(IffFile *iff, const char *chunk_id, IffChunk *group_chunk)
{
  if (IffReadNextChunk(iff, group_chunk) == 0) {
    return -1;
  }

  if (IffChunkMatch(group_chunk, chunk_id) == 0) {
    return -1;
  }

  return 0;
}

void IffReadChunkGroupEnd(IffFile *iff, IffChunk *group_chunk)
{
  IffSkipCurrentChunk(iff, group_chunk);
}

int IffPeekNextChunk(IffFile *iff, IffChunk *chunk)
{
  const DataSize curr = iff_tell(iff);

  strncpy(chunk->id, "", CHUNK_ID_SIZE);
  chunk->data_head = 0;
  chunk->data_size = 0;

  iff_read(iff, chunk->id, sizeof(chunk->id), 1);
  iff_seek(iff, curr, IFF_SEEK_SET);

  return 0;
}

int IffReadNextChildChunk(IffFile *iff, const IffChunk *parent, IffChunk *chunk)
{
  if (IffEndOfChunk(iff, parent)) {
    return 0;
  }

  return IffReadNextChunk(iff, chunk);
}

int IffReadNextChunk(IffFile *iff, IffChunk *chunk)
{
  strncpy(chunk->id, "", CHUNK_ID_SIZE);
  chunk->data_head = 0;
  chunk->data_size = 0;

  read_padding(iff);

  iff_read(iff, chunk->id, sizeof(chunk->id), 1);
  if (IffChunkMatch(chunk, "")) {
    return 0;
  }

  iff_read(iff, &chunk->data_size, sizeof(chunk->data_size), 1);
  chunk->data_head = iff_tell(iff);
  return 1;
}

void IffPutBackChunk(IffFile *iff, const IffChunk *chunk)
{
  const DataSize chunk_head = chunk->data_head - sizeof(DataSize) - sizeof(chunk->id);
  iff_seek(iff, chunk_head, IFF_SEEK_SET);
}

void IffSkipCurrentChunk(IffFile *iff, const IffChunk *chunk)
{
  const DataSize next_pos = chunk->data_head + chunk->data_size;
  iff_seek(iff, next_pos, IFF_SEEK_SET);
}

int IffChunkMatch(const IffChunk *chunk, const char *key)
{
  return strncmp(chunk->id, key, CHUNK_ID_SIZE) == 0 ? 1 : 0;
}

int IffEndOfChunk(const IffFile *iff, const IffChunk *chunk)
{
  const DataSize end_pos = chunk->data_head + chunk->data_size;
  const DataSize cur_pos = iff_tell(iff);

  return end_pos == cur_pos ? 1 : 0;
}
