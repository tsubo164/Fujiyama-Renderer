// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_framebuffer.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_color.h"

#include <cstddef>
#include <climits>
#include <cassert>

namespace fj {

FrameBuffer::FrameBuffer() :
    buf_(),
    width_(0),
    height_(0),
    nchannels_(0)
{
}

FrameBuffer::~FrameBuffer()
{
}

int FrameBuffer::GetWidth() const
{
  return width_;
}

int FrameBuffer::GetHeight() const
{
  return height_;
}

int FrameBuffer::GetChannelCount() const
{
  return nchannels_;
}

int FrameBuffer::GetSize() const
{
  return GetWidth() * GetHeight() * GetChannelCount();
}

void FrameBuffer::Resize(int width, int height, int nchannels)
{
  assert(width >= 0);
  assert(height >= 0);
  assert(nchannels >= 0);

  const int total_alloc = width * height * nchannels;
  // check overflow
  if (total_alloc < 0)
    return;

  std::vector<float> buftmp(total_alloc);
  if (buftmp.empty())
    return;

  // commit
  buf_.swap(buftmp);
  width_     = width;
  height_    = height;
  nchannels_ = nchannels;
}

bool FrameBuffer::IsEmpty() const
{
  return buf_.empty();
}

float *FrameBuffer::GetWritable(int x, int y, int z)
{
  if (!is_inside(x, y, z)) {
    return NULL;
  }
  return &buf_[get_index(x, y, z)];
}

const float *FrameBuffer::GetReadOnly(int x, int y, int z) const
{
  if (!is_inside(x, y, z)) {
    return NULL;
  }
  return &buf_[get_index(x, y, z)];
}

Color4 FrameBuffer::GetColor(int x, int y) const
{
  if (!is_inside(x, y, 0)) {
    return Color4();
  }

  const float *pixel = &buf_[get_index(x, y, 0)];
  const int channel_count = GetChannelCount();
  
  if (channel_count == 1) {
    return Color4(
        pixel[0],
        pixel[0],
        pixel[0],
        1);
  }
  else if (channel_count == 3) {
    return Color4(
        pixel[0],
        pixel[1],
        pixel[2],
        1);
  }
  else if (channel_count == 4) {
    return Color4(
        pixel[0],
        pixel[1],
        pixel[2],
        pixel[3]);
  }
  else {
    return Color4();
  }
}

void FrameBuffer::SetColor(int x, int y, const Color4 &rgba)
{
  float *pixel = GetWritable(x, y, 0);
  const int channel_count = GetChannelCount();

  if (pixel == NULL) {
    return;
  }
  
  if (channel_count == 1) {
    pixel[0] = rgba.r;
  }
  else if (channel_count == 3) {
    pixel[0] = rgba.r;
    pixel[1] = rgba.g;
    pixel[2] = rgba.b;
  }
  else if (channel_count == 4) {
    pixel[0] = rgba.r;
    pixel[1] = rgba.g;
    pixel[2] = rgba.b;
    pixel[3] = rgba.a;
  }
}

int FrameBuffer::get_index(int x, int y, int z) const
{
  return y * width_ * nchannels_ + x * nchannels_ + z;
}

bool FrameBuffer::is_inside(int x, int y, int z) const
{
  if (x < 0 || x >= GetWidth() ||
      y < 0 || y >= GetHeight() ||
      z < 0 || z >= GetChannelCount()) {
    return false;
  }
  return true;
}

void CopyInto(const FrameBuffer &src, FrameBuffer &dst,
    int dst_offsetx, int dst_offsety)
{
  for (int y = 0; y < dst.GetHeight(); y++) {
    for (int x = 0; x < dst.GetWidth(); x++) {
      const int src_x = x + dst_offsetx;
      const int src_y = y + dst_offsety;
      const Color4 color = src.GetColor(src_x, src_y);
      dst.SetColor(x, y, color);
    }
  }
}

void PasteInto(FrameBuffer &dst, const FrameBuffer &src,
    int src_offsetx, int src_offsety)
{
  for (int y = 0; y < src.GetHeight(); y++) {
    for (int x = 0; x < src.GetWidth(); x++) {
      const Color4 color = src.GetColor(x, y);
      const int dst_x = x + src_offsetx;
      const int dst_y = y + src_offsety;
      dst.SetColor(dst_x, dst_y, color);
    }
  }
}

} // namespace xxx
