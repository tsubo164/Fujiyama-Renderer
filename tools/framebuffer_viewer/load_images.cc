/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "load_images.h"
#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_mipmap.h"
#include "fj_box.h"

#include <string.h>

namespace fj {

const char *file_extension(const char *filename)
{
  const char *s = strrchr(filename, '.');
  if (s == NULL)
    return 0;

  return s + 1;
}

int load_fb(const char *filename, struct FrameBuffer *fb, struct BufferInfo *info)
{
  struct FbInput *in = NULL;

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

int load_mip(const char *filename, struct FrameBuffer *fb, struct BufferInfo *info)
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

  fb->Resize(in.width_, in.height_, in.nchannels_);

  {
    int x, y;
    struct FrameBuffer *tilebuf = FbNew();

    if (tilebuf == NULL) {
      /* TODO error handling */
    }
    tilebuf->Resize(in.tilesize_, in.tilesize_, in.nchannels_);

    for (y = 0; y < in.yntiles_; y++) {
      for (x = 0; x < in.xntiles_; x++) {
        int i;
        in.ReadTile(x, y, tilebuf->GetWritable(0, 0, 0));
        for (i = 0; i < in.tilesize_; i++) {
          float *dst;
          const float *src;
          dst = fb->GetWritable(x * in.tilesize_, y * in.tilesize_ + i, 0);
          src = tilebuf->GetReadOnly(0, i, 0);
          memcpy(dst, src, sizeof(float) * in.tilesize_ * in.nchannels_);
        }
      }
    }
    FbFree(tilebuf);
  }

  BOX2_SET(info->viewbox, 0, 0, in.width_, in.height_);
  BOX2_COPY(info->databox, info->viewbox);
  info->tilesize = in.tilesize_;

  return 0;
}

} // namespace xxx
