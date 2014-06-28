// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_IO_H
#define FJ_IO_H

#include "fj_compatibility.h"

#define NO_NAME NULL

namespace fj {

class InputFile;
class OutputFile;
class ChunkData;

class Vector;

enum ElementType {
  ELM_NONE = 0,
  ELM_INT,
  ELM_DOUBLE,
  ELM_VECTOR3
};

// ChunkData
FJ_API const char *ChkGetElementName(const ChunkData *chunk);
FJ_API int ChkGetElementType(const ChunkData *chunk);
FJ_API int ChkGetElementCount(const ChunkData *chunk);

// InputFile
FJ_API InputFile *IOOpenInputFile(const char *filename, const char *magic);
FJ_API void IOCloseInputFile(InputFile *in);

FJ_API int IOGetInputHeaderChunkCount(const InputFile *in);
FJ_API int IOGetInputDataChunkCount(const InputFile *in);
FJ_API const ChunkData *IOGetInputHeaderChunk(const InputFile *in, int index);
FJ_API const ChunkData *IOGetInputDataChunk(const InputFile *in, int index);
FJ_API int IOGetInputFileFormatVersion(const InputFile *in);

FJ_API void IOEndInputHeader(InputFile *in);
FJ_API int IOReadInputHeader(InputFile *in);
FJ_API int IOReadInputData(InputFile *in);

FJ_API void IOSetInputInt(InputFile *in,
    const char *element_name,
    int *dst_data,
    int element_count);

FJ_API void IOSetInputDouble(InputFile *in,
    const char *element_name,
    double *dst_data,
    int element_count);

FJ_API void IOSetInputVector3(InputFile *in,
    const char *element_name,
    Vector *dst_data,
    int element_count);

// OutputFile
FJ_API OutputFile *IOOpenOutputFile(const char *filename,
    const char *magic, int format_version);
FJ_API void IOCloseOutputFile(OutputFile *out);

FJ_API void IOEndOutputHeader(OutputFile *out);
FJ_API int IOWriteOutputHeader(OutputFile *out);
FJ_API int IOWriteOutputData(OutputFile *out);

FJ_API void IOSetOutputInt(OutputFile *out,
    const char *element_name,
    const int *src_data,
    int element_count);

FJ_API void IOSetOutputDouble(OutputFile *out,
    const char *element_name,
    const double *src_data,
    int element_count);

FJ_API void IOSetOutputVector3(OutputFile *out,
    const char *element_name,
    const Vector *src_data,
    int element_count);

} // namespace xxx

#endif // FJ_XXX_H
