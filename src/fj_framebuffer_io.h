// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_FRAMEBUFFERIO_H
#define FJ_FRAMEBUFFERIO_H

#include "fj_compatibility.h"
#include <stdio.h>

namespace fj {

class FrameBuffer;

class FJ_API FbInput {
public:
  FbInput() {}
  ~FbInput() {}

public:
  FILE *file;
  int32_t version;
  int32_t width;
  int32_t height;
  int32_t nchannels;

  int32_t viewbox[4];
  int32_t databox[4];

  float *data;
};

class FJ_API FbOutput {
public:
  FbOutput() {}
  ~FbOutput() {}

public:
  FILE *file;
  int32_t version;
  int32_t width;
  int32_t height;
  int32_t nchannels;

  int32_t viewbox[4];
  int32_t databox[4];

  const float *data;
};

enum FbErrorNo {
  ERR_FB_NOERR = 0,
  ERR_FB_NOMEM,
  ERR_FB_NOFILE,
  ERR_FB_NOTFB,
  ERR_FB_BADVER
};

FJ_API int FbGetErrorNo(void);
FJ_API const char *FbGetErrorMessage(int err);

FJ_API FbInput *FbOpenInputFile(const char *filename);
FJ_API void FbCloseInputFile(FbInput *in);
FJ_API int FbReadHeader(FbInput *in);
FJ_API int FbReadData(FbInput *in);

FJ_API FbOutput *FbOpenOutputFile(const char *filename);
FJ_API void FbCloseOutputFile(FbOutput *out);
FJ_API void FbWriteFile(FbOutput *out);

// high level interface for saving framebuffer file
FJ_API int FbSaveCroppedData(FrameBuffer *fb, const char *filename);

} // namespace xxx

#endif // FJ_XXX_H
