// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "load_images.h"
#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_mipmap.h"
#include "fj_box.h"

namespace fj {

std::string GetFileExtension(const std::string &filename)
{
  const size_t dotpos = filename.rfind('.');
  if(dotpos == std::string::npos){
    return std::string("");
  }

  return filename.substr(dotpos + 1, filename.size() - dotpos);
}

int LoadFb(const std::string &filename, FrameBuffer *fb, BufferInfo *info)
{
  if (fb == NULL) {
    return -1;
  }

  ReadFrameBuffer(filename, *fb);
  info->viewbox.min[0] = 0;
  info->viewbox.min[1] = 0;
  info->viewbox.max[0] = fb->GetWidth();
  info->viewbox.max[1] = fb->GetHeight();
  info->tilesize = 0;

  return 0;
}

int LoadMip(const std::string &filename, FrameBuffer *fb, BufferInfo *info)
{
  if (fb == NULL) {
    return -1;
  }

  MipInput in;

  in.Open(filename);
  if (!in.IsOpen()) {
    return -1;
  }

  if (in.ReadHeader()) {
    return -1;
  }

  fb->Resize(in.GetWidth(), in.GetHeight(), in.GetChannelCount());

  FrameBuffer tilebuf;
  tilebuf.Resize(in.GetTileSize(), in.GetTileSize(), in.GetChannelCount());

  for (int y = 0; y < in.GetTileCountY(); y++) {
    for (int x = 0; x < in.GetTileCountX(); x++) {
      in.ReadTile(x, y, tilebuf.GetWritable(0, 0, 0));
      const int start_x = x * in.GetTileSize();
      const int start_y = y * in.GetTileSize();
      PasteInto(*fb, tilebuf, start_x, start_y);
    }
  }

  info->viewbox.min[0] = 0;
  info->viewbox.min[1] = 0;
  info->viewbox.max[0] = in.GetWidth();
  info->viewbox.max[1] = in.GetHeight();
  info->tilesize = in.GetTileSize();

  return 0;
}

} // namespace xxx
