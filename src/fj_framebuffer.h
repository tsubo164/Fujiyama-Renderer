// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_FRAMEBUFFER_H
#define FJ_FRAMEBUFFER_H

#include "fj_compatibility.h"
#include <vector>

/* TODO delete BOX2 */
/* BOX2 */
/* BOX2[4] {min{0, 0}, max{0, 0}} */
#define BOX2_XSIZE(box) ((box)[2]-(box)[0])
#define BOX2_YSIZE(box) ((box)[3]-(box)[1])

#define BOX2_SET(dst,xmin,ymin,xmax,ymax) do { \
  (dst)[0] = (xmin); \
  (dst)[1] = (ymin); \
  (dst)[2] = (xmax); \
  (dst)[3] = (ymax); \
  } while(0)

#define BOX2_COPY(dst,a) do { \
  (dst)[0] = (a)[0]; \
  (dst)[1] = (a)[1]; \
  (dst)[2] = (a)[2]; \
  (dst)[3] = (a)[3]; \
  } while(0)

namespace fj {

class Color4;

class FJ_API FrameBuffer {
public:
  FrameBuffer();
  ~FrameBuffer();

  int GetWidth() const;
  int GetHeight() const;
  int GetChannelCount() const;

  // Returns the head of buffer if resize is done, otherwise NULL.
  float *Resize(int width, int height, int nchannels);

  // Returns 0 if fb is has rgba channels otherwise -1.
  // Bounds can be computed only when fb is an rgba buffer.
  int ComputeBounds(int *bounds);
  int IsEmpty() const;

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

FJ_API void Copy(FrameBuffer &dst, const FrameBuffer &src,
    int src_offsetx, int src_offsety);

} // namespace xxx

#endif // FJ_XXX_H
