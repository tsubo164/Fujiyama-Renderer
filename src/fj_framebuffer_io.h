// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_FRAMEBUFFER_IO_H
#define FJ_FRAMEBUFFER_IO_H

#include "fj_compatibility.h"
#include <string>

namespace fj {

class FrameBuffer;

enum FbErrorNo {
  ERR_FB_NOERR = 0,
  ERR_FB_NOMEM,
  ERR_FB_NOFILE,
  ERR_FB_NOTFB,
  ERR_FB_BADVER
};

FJ_API int FbGetErrorNo(void);
FJ_API const char *FbGetErrorMessage(int err);

FJ_API int WriteFrameBuffer(const std::string &filename, const FrameBuffer &fb);
FJ_API int ReadFrameBuffer(const std::string &filename, FrameBuffer &fb);

} // namespace xxx

#endif // FJ_XXX_H
