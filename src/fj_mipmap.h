/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef MIPMAP_H
#define MIPMAP_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct FrameBuffer;

struct MipInput {
  FILE *file;
  int version;
  int width;
  int height;
  int nchannels;
  float *data;

  int tilesize;
  int xntiles;
  int yntiles;

  size_t offset_of_header;
  size_t offset_of_tile;
};

struct MipOutput {
  FILE *file;
  int version;
  int width;
  int height;
  int nchannels;
  int tilesize;

  struct FrameBuffer *fb;
};

enum MipErrorNo {
  ERR_MIP_NOERR = 0,
  ERR_MIP_NOMEM,
  ERR_MIP_NOFILE,
  ERR_MIP_NOTMIP,
  ERR_MIP_BADVER
};

extern int MipGetErrorNo(void);
extern const char *MipGetErrorMessage(int err);

/* MipInput */
extern struct MipInput *MipOpenInputFile(const char *filename);
extern void MipCloseInputFile(struct MipInput *in);

extern int MipReadHeader(struct MipInput *in);
extern int MipReadTile(struct MipInput *in, int xtile, int ytile);

/* MipOutput */
extern struct MipOutput *MipOpenOutputFile(const char *filename);
extern void MipCloseOutputFile(struct MipOutput *out);

extern int MipGenerateFromSourceData(struct MipOutput *out,
    const float *pixels, int width, int height, int nchannels);
extern void MipWriteFile(struct MipOutput *out);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

