// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef LOAD_IMAGES_H
#define LOAD_IMAGES_H

#include "fj_rectangle.h"
#include <string>

namespace fj {

class FrameBuffer;

class BufferInfo {
public:
  BufferInfo() : viewbox(), tilesize(0) {}
  ~BufferInfo() {}

public:
  Rectangle viewbox;
  int tilesize;
};

extern std::string GetFileExtension(const std::string &filename);

extern int LoadFb(const std::string &filename, FrameBuffer *fb, BufferInfo *info);
extern int LoadMip(const std::string &filename, FrameBuffer *fb, BufferInfo *info);

} // namespace xxx

#endif // XXX_H
