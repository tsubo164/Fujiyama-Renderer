// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_MIPMAP_H
#define FJ_MIPMAP_H

#include <string>
#include <cstdio>

namespace fj {

class FrameBuffer;

class MipInput {
public:
  MipInput();
  ~MipInput();

  int Open(const std::string &filename);
  void Close();

  int ReadHeader();
  int ReadTile(int xtile, int ytile, float *dst);

  bool IsOpen() const;

public:
  FILE *file_;
  int version_;
  int width_;
  int height_;
  int nchannels_;

  int tilesize_;
  int xntiles_;
  int yntiles_;

  size_t offset_of_header_;
  size_t offset_of_tile_;
};

class MipOutput {
public:
  MipOutput() {}
  ~MipOutput() {}

public:
  FILE *file;
  int version;
  int width;
  int height;
  int nchannels;
  int tilesize;

  FrameBuffer *fb;
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

// MipInput
// extern MipInput *MipOpenInputFile(const char *filename);
// extern void MipCloseInputFile(MipInput *in);

// extern int MipReadHeader(MipInput *in);
// extern int MipReadTile(MipInput *in, int xtile, int ytile);

// MipOutput
extern MipOutput *MipOpenOutputFile(const char *filename);
extern void MipCloseOutputFile(MipOutput *out);

extern int MipGenerateFromSourceData(MipOutput *out,
    const float *pixels, int width, int height, int nchannels);
extern void MipWriteFile(MipOutput *out);

} // namespace xxx

#endif // FJ_XXX_H
