// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef LOAD_IMAGES_H
#define LOAD_IMAGES_H

#include <string>

namespace fj {

class FrameBuffer;

class BufferInfo {
public:
  BufferInfo() : databox(), viewbox(), tilesize(0) {}
  ~BufferInfo() {}

public:
  int databox[4];
  int viewbox[4];
  int tilesize;
};

extern std::string file_extension(const std::string &filename);

extern int load_fb(const char *filename, FrameBuffer *fb, BufferInfo *info);

extern int load_mip(const char *filename, FrameBuffer *fb, BufferInfo *info);

} // namespace xxx

#endif // XXX_H
