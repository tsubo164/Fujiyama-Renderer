// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_FRAMEBUFFER_H
#define FJ_FRAMEBUFFER_H

#include "fj_compatibility.h"
#include <vector>

namespace fj {

class Color4;

class FJ_API FrameBuffer {
public:
  FrameBuffer();
  ~FrameBuffer();

  int GetWidth() const;
  int GetHeight() const;
  int GetChannelCount() const;

  int GetSize() const;
  void Resize(int width, int height, int nchannels);
  bool IsEmpty() const;

  // Use these functions with caution.
  // Returns NULL if (x, y, z) is out of bounds.
  float *GetWritable(int x, int y, int z);
  const float *GetReadOnly(int x, int y, int z) const;

  // Get color at pixel (x, y).
  // (r, r, r, 1) will be returned when framebuffer is grayscale
  // (r, g, b, 1) will be returned when framebuffer is rgb
  // (r, g, b, a) will be returned when framebuffer is rgba
  Color4 GetColor(int x, int y) const;

  // Set color at pixel (x, y).
  // (r)          will be set when framebuffer is grayscale
  // (r, g, b)    will be set when framebuffer is rgb
  // (r, g, b, a) will be set when framebuffer is rgba
  void SetColor(int x, int y, const Color4 &rgba);

private:
  int get_index(int x, int y, int z) const;
  bool is_inside(int x, int y, int z) const;

  std::vector<float> buf_;
  int width_;
  int height_;
  int nchannels_;
};

FJ_API void CopyInto(const FrameBuffer &src, FrameBuffer &dst,
    int dst_offsetx, int dst_offsety);

FJ_API void PasteInto(FrameBuffer &dst, const FrameBuffer &src,
    int src_offsetx, int src_offsety);

} // namespace xxx

#endif // FJ_XXX_H
