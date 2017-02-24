// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "load_images.h"
#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_mipmap.h"
#include "fj_box.h"

#include <cstring>

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
  if (fb == NULL)
    return -1;

  FbInput *in = FbOpenInputFile(filename.c_str());
  if (in == NULL) {
    return -1;
  }

  if (FbReadHeader(in)) {
    FbCloseInputFile(in);
    return -1;
  }

  fb->Resize(in->width, in->height, in->nchannels);
  in->data = fb->GetWritable(0, 0, 0);
  FbReadData(in);

  BOX2_COPY(info->viewbox, in->viewbox);
  BOX2_COPY(info->databox, in->databox);
  info->tilesize = 0;

  FbCloseInputFile(in);

  return 0;
}

int LoadMip(const std::string &filename, FrameBuffer *fb, BufferInfo *info)
{
  if (fb == NULL)
    return -1;

  MipInput in;

  in.Open(filename);
  if (!in.IsOpen())
    return -1;

  if (in.ReadHeader()) {
    return -1;
  }

  fb->Resize(in.GetWidth(), in.GetHeight(), in.GetChannelCount());

  FrameBuffer tilebuf;
  tilebuf.Resize(in.GetTileSize(), in.GetTileSize(), in.GetChannelCount());

  for (int y = 0; y < in.GetTileCountY(); y++) {
    for (int x = 0; x < in.GetTileCountX(); x++) {
      in.ReadTile(x, y, tilebuf.GetWritable(0, 0, 0));
      for (int i = 0; i < in.GetTileSize(); i++) {
        float *dst;
        const float *src;
        dst = fb->GetWritable(x * in.GetTileSize(), y * in.GetTileSize() + i, 0);
        src = tilebuf.GetReadOnly(0, i, 0);
        memcpy(dst, src, sizeof(float) * in.GetTileSize() * in.GetChannelCount());
      }
    }
  }

  BOX2_SET(info->viewbox, 0, 0, in.GetWidth(), in.GetHeight());
  BOX2_COPY(info->databox, info->viewbox);
  info->tilesize = in.GetTileSize();

  return 0;
}

} // namespace xxx
