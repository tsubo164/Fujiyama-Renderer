// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_IO_H
#define FJ_IO_H

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
extern const char *ChkGetElementName(const ChunkData *chunk);
extern int ChkGetElementType(const ChunkData *chunk);
extern int ChkGetElementCount(const ChunkData *chunk);

// InputFile
extern InputFile *IOOpenInputFile(const char *filename, const char *magic);
extern void IOCloseInputFile(InputFile *in);

extern int IOGetInputHeaderChunkCount(const InputFile *in);
extern int IOGetInputDataChunkCount(const InputFile *in);
extern const ChunkData *IOGetInputHeaderChunk(const InputFile *in, int index);
extern const ChunkData *IOGetInputDataChunk(const InputFile *in, int index);
extern int IOGetInputFileFormatVersion(const InputFile *in);

extern void IOEndInputHeader(InputFile *in);
extern int IOReadInputHeader(InputFile *in);
extern int IOReadInputData(InputFile *in);

extern void IOSetInputInt(InputFile *in,
    const char *element_name,
    int *dst_data,
    int element_count);

extern void IOSetInputDouble(InputFile *in,
    const char *element_name,
    double *dst_data,
    int element_count);

extern void IOSetInputVector3(InputFile *in,
    const char *element_name,
    Vector *dst_data,
    int element_count);

// OutputFile
extern OutputFile *IOOpenOutputFile(const char *filename,
    const char *magic, int format_version);
extern void IOCloseOutputFile(OutputFile *out);

extern void IOEndOutputHeader(OutputFile *out);
extern int IOWriteOutputHeader(OutputFile *out);
extern int IOWriteOutputData(OutputFile *out);

extern void IOSetOutputInt(OutputFile *out,
    const char *element_name,
    const int *src_data,
    int element_count);

extern void IOSetOutputDouble(OutputFile *out,
    const char *element_name,
    const double *src_data,
    int element_count);

extern void IOSetOutputVector3(OutputFile *out,
    const char *element_name,
    const Vector *src_data,
    int element_count);

} // namespace xxx

#endif // FJ_XXX_H
