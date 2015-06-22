// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_io.h"
#include "fj_vector.h"

#include <vector>
#include <cstdio>
#include <cstring>

namespace fj {

typedef const char *src_ptr;
typedef char *dst_ptr;

typedef unsigned char ElmType;
typedef unsigned char NameSize;

enum { MAGIC_SIZE = 4 };
enum { ELEMENT_NAME_SIZE = 64 };

class ChunkData {
public:
  ChunkData() :
      element_name(),
      element_type(ELM_NONE),
      element_count(0),
      src_data(NULL),
      dst_data(NULL)
  {
  }
  ~ChunkData() {}

public:
  char element_name[ELEMENT_NAME_SIZE];
  ElmType element_type;
  int element_count;

  src_ptr src_data;
  dst_ptr dst_data;
};

static ChunkData input_chunk_data(
    void *dst_data,
    const char *element_name,
    ElmType element_type,
    int element_count)
{
  const size_t len = strlen(element_name);
  ChunkData chunk;

  if (len > ELEMENT_NAME_SIZE - 1) {
    return chunk;
  }

  strncpy(chunk.element_name, element_name, ELEMENT_NAME_SIZE);
  chunk.element_count = element_count;
  chunk.element_type = element_type;
  chunk.dst_data = (dst_ptr) dst_data;

  return chunk;
}

static ChunkData output_chunk_data(
    const void *src_data,
    const char *element_name,
    ElmType element_type,
    int element_count)
{
  const size_t len = strlen(element_name);
  ChunkData chunk;

  if (len > ELEMENT_NAME_SIZE - 1) {
    return chunk;
  }

  strncpy(chunk.element_name, element_name, ELEMENT_NAME_SIZE);
  chunk.element_count = element_count;
  chunk.element_type = element_type;
  chunk.src_data = (src_ptr) src_data;

  return chunk;
}

// ChunkData
const char *ChkGetElementName(const ChunkData *chunk)
{
  return chunk->element_name;
}

int ChkGetElementType(const ChunkData *chunk)
{
  return chunk->element_type;
}

int ChkGetElementCount(const ChunkData *chunk)
{
  return chunk->element_count;
}

typedef unsigned char Endian;
typedef unsigned char FmtVer;
enum { IO_LITTLE_ENDIAN = 0x00, IO_BIG_ENDIAN = 0x01 };
#define INFO_N_PADDINGS (12-2)
#define INFO_PADDING '*'
class FileInfo {
public:
  FileInfo() {}
  ~FileInfo() {}

public:
  char magic[MAGIC_SIZE];
  Endian endian;
  FmtVer fmtver;
};
static int copy_magic_number(char *dst, const char *src);

static int read_file_info(FILE *file, FileInfo *info);
static int write_file_info(FILE *file, FileInfo *info);

static int read_chunk_info(FILE *file, ChunkData *chunk);
static int write_chunk_info(FILE *file, const ChunkData *chunk);

static int read_chunk_data(FILE *file, ChunkData *chunk);
static int write_chunk_data(FILE *file, const ChunkData *chunk);

// InputFile
class InputFile {
public:
  InputFile() {}
  ~InputFile() {}

public:
  FILE *file;
  FileInfo info;

  std::vector<ChunkData> header_chunks;
  std::vector<ChunkData> data_chunks;
  int is_header_ended;
};

InputFile *IOOpenInputFile(const char *filename, const char *magic)
{
  int err = 0;
  InputFile *in = new InputFile();

  if (in == NULL) {
    return NULL;
  }

  in->file = fopen(filename, "rb");
  if (in->file == NULL) {
    delete in;
    return NULL;
  }

  in->is_header_ended = 0;

  err = read_file_info(in->file, &in->info);
  if (err) {
    IOCloseInputFile(in);
    return NULL;
  }
  if (memcmp(in->info.magic, magic, MAGIC_SIZE) != 0) {
    IOCloseInputFile(in);
    return NULL;
  }

  return in;
}

void IOCloseInputFile(InputFile *in)
{
  if (in == NULL) {
    return;
  }
  
  if (in->file != NULL) {
    fclose(in->file);
  }

  delete in;
}

int IOGetInputHeaderChunkCount(const InputFile *in)
{
  return in->header_chunks.size();
}

int IOGetInputDataChunkCount(const InputFile *in)
{
  return in->data_chunks.size();
}

const ChunkData *IOGetInputHeaderChunk(const InputFile *in, int index)
{
  if (index < 0 || index > IOGetInputHeaderChunkCount(in)) {
    return NULL;
  }
  return &in->header_chunks[index];
}

const ChunkData *IOGetInputDataChunk(const InputFile *in, int index)
{
  if (index < 0 || index >= IOGetInputDataChunkCount(in)) {
    return NULL;
  }
  return &in->data_chunks[index];
}

int IOGetInputFileFormatVersion(const InputFile *in)
{
  return in->info.fmtver;
}

void IOEndInputHeader(InputFile *in)
{
  in->is_header_ended = 1;
}

int IOReadInputHeader(InputFile *in)
{
  int header_chunk_count = 0;
  int data_chunk_count = 0;
  size_t nreads = 0;
  int i;

  nreads = fread(&header_chunk_count, sizeof(header_chunk_count), 1, in->file);
  if (nreads != 1) {
    // TODO error handling
    return -1;
  }

  nreads = fread(&data_chunk_count, sizeof(data_chunk_count), 1, in->file);
  if (nreads != 1) {
    // TODO error handling
    return -1;
  }

  for (i = 0; i < IOGetInputHeaderChunkCount(in); i++) {
    ChunkData *chunk = &in->header_chunks[i];
    read_chunk_data(in->file, chunk);
  }

  for (i = 0; i < data_chunk_count; i++) {
    ChunkData chunk;
    read_chunk_info(in->file, &chunk);
    in->data_chunks.push_back(chunk);
  }

  return 0;
}

int IOReadInputData(InputFile *in)
{
  int data_chunk_count = 0;
  int i;

  if (in == NULL) {
    return -1;
  }

  if (in->file == NULL) {
    return -1;
  }

  data_chunk_count = IOGetInputDataChunkCount(in);

  for (i = 0; i < data_chunk_count; i++) {
    ChunkData *chunk = &in->data_chunks[i];
    read_chunk_data(in->file, chunk);
  }

  return 0;
}

// OutputFile
class OutputFile {
public:
  OutputFile() {}
  ~OutputFile() {}

public:
  FILE *file;
  FileInfo info;

  std::vector<ChunkData> header_chunks;
  std::vector<ChunkData> data_chunks;
  int is_header_ended;
  int is_written;
};

OutputFile *IOOpenOutputFile(const char *filename,
    const char *magic, int format_version)
{
  OutputFile *out = new OutputFile();

  if (out == NULL) {
    return NULL;
  }

  out->file = fopen(filename, "wb");
  if (out->file == NULL) {
    delete out;
    return NULL;
  }

  out->is_header_ended = 0;
  out->is_written = 0;

  if (copy_magic_number(out->info.magic, magic)) {
    IOCloseOutputFile(out);
    return NULL;
  }
  // TODO detect endian
  out->info.endian = IO_LITTLE_ENDIAN;
  out->info.fmtver = format_version;
  if (format_version > 127) {
    IOCloseOutputFile(out);
    return NULL;
  }

  return out;
}

void IOCloseOutputFile(OutputFile *out)
{
  if (out == NULL) {
    return;
  }

  if (out->is_written == 0) {
    IOWriteOutputHeader(out);
  }

  if (out->file != NULL) {
    fclose(out->file);
  }

  delete out;
}

void IOEndOutputHeader(OutputFile *out)
{
  out->is_header_ended = 1;
}

int IOWriteOutputHeader(OutputFile *out)
{
  int header_chunk_count = 0;
  int data_chunk_count = 0;
  int err = 0;
  int i;

  if (out == NULL) {
    return -1;
  }

  if (out->file == NULL) {
    return -1;
  }

  err = write_file_info(out->file, &out->info);
  if (err) {
    return -1;
  }

  header_chunk_count = out->header_chunks.size();
  fwrite(&header_chunk_count, sizeof(header_chunk_count), 1, out->file);

  data_chunk_count = out->data_chunks.size();
  fwrite(&data_chunk_count, sizeof(data_chunk_count), 1, out->file);

  for (i = 0; i < header_chunk_count; i++) {
    const ChunkData *chunk = &out->header_chunks[i];
    write_chunk_data(out->file, chunk);
  }

  for (i = 0; i < data_chunk_count; i++) {
    const ChunkData *chunk = &out->data_chunks[i];
    write_chunk_info(out->file, chunk);
  }

  out->is_written = 1;

  return 0;
}

int IOWriteOutputData(OutputFile *out)
{
  int data_chunk_count = 0;
  int i;

  if (out == NULL) {
    return -1;
  }

  if (out->file == NULL) {
    return -1;
  }

  data_chunk_count = out->data_chunks.size();

  for (i = 0; i < data_chunk_count; i++) {
    const ChunkData *chunk = &out->data_chunks[i];
    write_chunk_data(out->file, chunk);
  }

  return 0;
}

void IOSetInputInt(InputFile *in,
    const char *element_name,
    int *dst_data,
    int element_count)
{
  if (in->is_header_ended) {
    int i;
    for (i = 0; i < IOGetInputDataChunkCount(in); i++) {
      ChunkData *chk = &in->data_chunks[i];
      if (strcmp(chk->element_name, element_name) == 0) {
        chk->dst_data = (dst_ptr) dst_data;
        break;
      }
    }
  } else {
    const ChunkData chunk = input_chunk_data(
        dst_data,
        element_name,
        ELM_INT,
        element_count);
    in->header_chunks.push_back(chunk);
  }
}

void IOSetInputDouble(InputFile *in,
    const char *element_name,
    double *dst_data,
    int element_count)
{
  if (in->is_header_ended) {
    int i;
    for (i = 0; i < IOGetInputDataChunkCount(in); i++) {
      ChunkData *chk = &in->data_chunks[i];
      if (strcmp(chk->element_name, element_name) == 0) {
        chk->dst_data = (dst_ptr) dst_data;
        return;
      }
    }
  } else {
    const ChunkData chunk = input_chunk_data(
        dst_data,
        element_name,
        ELM_DOUBLE,
        element_count);
    in->header_chunks.push_back(chunk);
  }
}

void IOSetInputVector3(InputFile *in,
    const char *element_name,
    Vector *dst_data,
    int element_count)
{
  if (in->is_header_ended) {
    int i;
    for (i = 0; i < IOGetInputDataChunkCount(in); i++) {
      ChunkData *chk = &in->data_chunks[i];
      if (strcmp(chk->element_name, element_name) == 0) {
        chk->dst_data = (dst_ptr) dst_data;
        break;
      }
    }
  } else {
    const ChunkData chunk = input_chunk_data(
        dst_data,
        element_name,
        ELM_VECTOR3,
        element_count);
    in->header_chunks.push_back(chunk);
  }
}

void IOSetOutputInt(OutputFile *out,
    const char *element_name,
    const int *src_data,
    int element_count)
{
  const ChunkData chunk = output_chunk_data(
      src_data,
      element_name,
      ELM_INT,
      element_count);

  if (out->is_header_ended) {
    out->data_chunks.push_back(chunk);
  } else {
    out->header_chunks.push_back(chunk);
  }
}

void IOSetOutputDouble(OutputFile *out,
    const char *element_name,
    const double *src_data,
    int element_count)
{
  const ChunkData chunk = output_chunk_data(
      src_data,
      element_name,
      ELM_DOUBLE,
      element_count);

  if (out->is_header_ended) {
    out->data_chunks.push_back(chunk);
  } else {
    out->header_chunks.push_back(chunk);
  }
}

void IOSetOutputVector3(OutputFile *out,
    const char *element_name,
    const Vector *src_data,
    int element_count)
{
  const ChunkData chunk = output_chunk_data(
      src_data,
      element_name,
      ELM_VECTOR3,
      element_count);

  if (out->is_header_ended) {
    out->data_chunks.push_back(chunk);
  } else {
    out->header_chunks.push_back(chunk);
  }
}

static int copy_magic_number(char *dst, const char *src)
{
  const size_t len = strlen(src);

  if (len != MAGIC_SIZE) {
    return -1;
  }

  strncpy(dst, src, MAGIC_SIZE);
  return 0;
}

static int read_file_info(FILE *file, FileInfo *info)
{
  size_t nreads = 0;
  int err = 0;

  nreads = fread(info->magic, sizeof(char), MAGIC_SIZE, file);
  if (nreads != MAGIC_SIZE) {
    return -1;
  }

  nreads = fread(&info->endian, sizeof(info->endian), 1, file);
  if (nreads != 1) {
    return -1;
  }

  nreads = fread(&info->fmtver, sizeof(info->fmtver), 1, file);
  if (nreads != 1) {
    return -1;
  }

  // skip padding
  err = fseek(file, INFO_N_PADDINGS, SEEK_CUR);
  if (err) {
    // TODO error handling
    return -1;
  }

  return 0;
}

static int write_file_info(FILE *file, FileInfo *info)
{
  size_t nwrotes = 0;
  int i;

  nwrotes = fwrite(info->magic, sizeof(char), MAGIC_SIZE, file);
  if (nwrotes != MAGIC_SIZE) {
    return -1;
  }

  nwrotes = fwrite(&info->endian, sizeof(info->endian), 1, file);
  if (nwrotes != 1) {
    // TODO error handling
    return -1;
  }

  nwrotes = fwrite(&info->fmtver, sizeof(info->fmtver), 1, file);
  if (nwrotes != 1) {
    // TODO error handling
    return -1;
  }

  for (i = 0; i < INFO_N_PADDINGS; i++) {
    char c = INFO_PADDING;
    nwrotes = fwrite(&c, sizeof(c), 1, file);
    if (nwrotes != 1) {
      // TODO error handling
      return -1;
    }
  }

  return 0;
}

static int read_chunk_info(FILE *file, ChunkData *chunk)
{
  size_t nreads = 0;
  NameSize namesize = 0;

  nreads = fread(&namesize, sizeof(namesize), 1, file);
  if (nreads != 1) {
    // TODO error handling
    return -1;
  }
  nreads = fread(chunk->element_name, sizeof(char), namesize, file);
  if (nreads != namesize) {
    // TODO error handling
    return -1;
  }
  nreads = fread(&chunk->element_type, sizeof(chunk->element_type), 1, file);
  if (nreads != 1) {
    // TODO error handling
    return -1;
  }
  nreads = fread(&chunk->element_count, sizeof(chunk->element_count), 1, file);
  if (nreads != 1) {
    // TODO error handling
    return -1;
  }

  return 0;
}

static int write_chunk_info(FILE *file, const ChunkData *chunk)
{
  const NameSize namesize = (NameSize) strlen(chunk->element_name) + 1;

  fwrite(&namesize, sizeof(namesize), 1, file);
  fwrite(chunk->element_name, sizeof(char), namesize, file);
  fwrite(&chunk->element_type, sizeof(chunk->element_type), 1, file);
  fwrite(&chunk->element_count, sizeof(chunk->element_count), 1, file);

  return 0;
}

static int read_chunk_data(FILE *file, ChunkData *chunk)
{
  const int NELEMS = chunk->element_count;
  int i;

  if (chunk->dst_data == NULL) {
    return -1;
  }

  switch (chunk->element_type) {

  case ELM_INT: {
    size_t nreads = 0;
    int *dst_data = (int *) chunk->dst_data;
    for (i = 0; i < NELEMS; i++) {
      nreads = fread(&dst_data[i], sizeof(*dst_data), 1, file);
      if (nreads != 1) {
        // TODO error handling
        return -1;
      }
    }
    }
    break;

  case ELM_DOUBLE: {
    size_t nreads = 0;
    double *dst_data = (double *) chunk->dst_data;
    for (i = 0; i < NELEMS; i++) {
      nreads = fread(&dst_data[i], sizeof(*dst_data), 1, file);
      if (nreads != 1) {
        // TODO error handling
        return -1;
      }
    }
    }
    break;

  case ELM_VECTOR3: {
    size_t nreads = 0;
    Vector *dst_data = (Vector *) chunk->dst_data;
    for (i = 0; i < NELEMS; i++) {
      nreads = fread(&dst_data[i].x, sizeof(dst_data[i].x), 1, file);
      if (nreads != 1) {
        // TODO error handling
        return -1;
      }
      nreads = fread(&dst_data[i].y, sizeof(dst_data[i].y), 1, file);
      if (nreads != 1) {
        // TODO error handling
        return -1;
      }
      nreads = fread(&dst_data[i].z, sizeof(dst_data[i].z), 1, file);
      if (nreads != 1) {
        // TODO error handling
        return -1;
      }
    }
    }
    break;

  default:
    break;
  }

  return 0;
}

static int write_chunk_data(FILE *file, const ChunkData *chunk)
{
  const int NELEMS = chunk->element_count;
  int i;

  switch (chunk->element_type) {

  case ELM_INT: {
    const int *src_data = (const int *) chunk->src_data;
    for (i = 0; i < NELEMS; i++) {
      fwrite(&src_data[i], sizeof(*src_data), 1, file);
    }
    }
    break;

  case ELM_DOUBLE: {
    const double *src_data = (const double *) chunk->src_data;
    for (i = 0; i < NELEMS; i++) {
      fwrite(&src_data[i], sizeof(*src_data), 1, file);
    }
    }
    break;

  case ELM_VECTOR3: {
    const Vector *src_data = (const Vector *) chunk->src_data;
    for (i = 0; i < NELEMS; i++) {
      fwrite(&src_data[i].x, sizeof(src_data[i].x), 1, file);
      fwrite(&src_data[i].y, sizeof(src_data[i].y), 1, file);
      fwrite(&src_data[i].z, sizeof(src_data[i].z), 1, file);
    }
    }
    break;

  default:
    break;
  }

  return 0;
}

} // namespace xxx
