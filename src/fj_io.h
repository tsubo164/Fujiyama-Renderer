/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_IO_H
#define FJ_IO_H

#define NO_NAME NULL

#ifdef __cplusplus
extern "C" {
#endif

struct InputFile;
struct OutputFile;
struct ChunkData;

struct Vector;

enum ElementType {
  ELM_NONE = 0,
  ELM_INT,
  ELM_DOUBLE,
  ELM_VECTOR3
};

/* ChunkData */
extern const char *ChkGetElementName(const struct ChunkData *chunk);
extern int ChkGetElementType(const struct ChunkData *chunk);
extern int ChkGetElementCount(const struct ChunkData *chunk);

/* InputFile */
extern struct InputFile *IOOpenInputFile(const char *filename, const char *magic);
extern void IOCloseInputFile(struct InputFile *in);

extern int IOGetInputHeaderChunkCount(const struct InputFile *in);
extern int IOGetInputDataChunkCount(const struct InputFile *in);
extern const struct ChunkData *IOGetInputHeaderChunk(const struct InputFile *in, int index);
extern const struct ChunkData *IOGetInputDataChunk(const struct InputFile *in, int index);
extern int IOGetInputFileFormatVersion(const struct InputFile *in);

extern void IOEndInputHeader(struct InputFile *in);
extern int IOReadInputHeader(struct InputFile *in);
extern int IOReadInputData(struct InputFile *in);

extern void IOSetInputInt(struct InputFile *in,
    const char *element_name,
    int *dst_data,
    int element_count);

extern void IOSetInputDouble(struct InputFile *in,
    const char *element_name,
    double *dst_data,
    int element_count);

extern void IOSetInputVector3(struct InputFile *in,
    const char *element_name,
    struct Vector *dst_data,
    int element_count);

/* OutputFile */
extern struct OutputFile *IOOpenOutputFile(const char *filename,
    const char *magic, int format_version);
extern void IOCloseOutputFile(struct OutputFile *out);

extern void IOEndOutputHeader(struct OutputFile *out);
extern int IOWriteOutputHeader(struct OutputFile *out);
extern int IOWriteOutputData(struct OutputFile *out);

extern void IOSetOutputInt(struct OutputFile *out,
    const char *element_name,
    const int *src_data,
    int element_count);

extern void IOSetOutputDouble(struct OutputFile *out,
    const char *element_name,
    const double *src_data,
    int element_count);

extern void IOSetOutputVector3(struct OutputFile *out,
    const char *element_name,
    const struct Vector *src_data,
    int element_count);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
