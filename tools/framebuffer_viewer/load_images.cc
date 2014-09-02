// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "load_images.h"
#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_mipmap.h"
#include "fj_box.h"

#include <string.h>

namespace fj {

std::string file_extension(const std::string &filename)
{

  const size_t dotpos = filename.rfind('.');

  if(dotpos == std::string::npos){
    return std::string("");
  }

  return filename.substr(dotpos + 1, filename.size() - dotpos);
}

int load_fb(const char *filename, FrameBuffer *fb, BufferInfo *info)
{
  FbInput *in = NULL;

  if (fb == NULL)
    return -1;

  in = FbOpenInputFile(filename);
  if (in == NULL)
    return -1;

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

int load_mip(const char *filename, FrameBuffer *fb, BufferInfo *info)
{
  MipInput in;

  if (fb == NULL)
    return -1;

  in.Open(filename);
  if (!in.IsOpen())
    return -1;

  if (in.ReadHeader()) {
    return -1;
  }

  fb->Resize(in.GetWidth(), in.GetHeight(), in.GetChannelCount());

  {
    int x, y;
    FrameBuffer tilebuf;

    tilebuf.Resize(in.GetTileSize(), in.GetTileSize(), in.GetChannelCount());

    for (y = 0; y < in.GetTileCountY(); y++) {
      for (x = 0; x < in.GetTileCountX(); x++) {
        int i;
        in.ReadTile(x, y, tilebuf.GetWritable(0, 0, 0));
        for (i = 0; i < in.GetTileSize(); i++) {
          float *dst;
          const float *src;
          dst = fb->GetWritable(x * in.GetTileSize(), y * in.GetTileSize() + i, 0);
          src = tilebuf.GetReadOnly(0, i, 0);
          memcpy(dst, src, sizeof(float) * in.GetTileSize() * in.GetChannelCount());
        }
      }
    }
  }

  BOX2_SET(info->viewbox, 0, 0, in.GetWidth(), in.GetHeight());
  BOX2_COPY(info->databox, info->viewbox);
  info->tilesize = in.GetTileSize();

  return 0;
}

} // namespace xxx
