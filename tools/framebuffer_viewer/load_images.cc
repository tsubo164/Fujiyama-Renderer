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

  FbResize(fb, in->width, in->height, in->nchannels);
  in->data = FbGetWritable(fb, 0, 0, 0);
  FbReadData(in);

  BOX2_COPY(info->viewbox, in->viewbox);
  BOX2_COPY(info->databox, in->databox);
  info->tilesize = 0;

  FbCloseInputFile(in);

  return 0;
}

int load_mip(const char *filename, struct FrameBuffer *fb, struct BufferInfo *info)
{
  struct MipInput *in = NULL;

  if (fb == NULL)
    return -1;

  in = MipOpenInputFile(filename);
  if (in == NULL)
    return -1;

  if (MipReadHeader(in)) {
    MipCloseInputFile(in);
    return -1;
  }

  FbResize(fb, in->width, in->height, in->nchannels);

  {
    int x, y;
    struct FrameBuffer *tilebuf = FbNew();

    if (tilebuf == NULL) {
      /* TODO error handling */
    }
    FbResize(tilebuf, in->tilesize, in->tilesize, in->nchannels);

    for (y = 0; y < in->yntiles; y++) {
      for (x = 0; x < in->xntiles; x++) {
        int i;
        in->data = FbGetWritable(tilebuf, 0, 0, 0);
        MipReadTile(in, x, y);
        for (i = 0; i < in->tilesize; i++) {
          float *dst;
          const float *src;
          dst = FbGetWritable(fb, x * in->tilesize, y * in->tilesize + i, 0);
          src = FbGetReadOnly(tilebuf, 0, i, 0);
          memcpy(dst, src, sizeof(float) * in->tilesize * in->nchannels);
        }
      }
    }
    FbFree(tilebuf);
  }

  BOX2_SET(info->viewbox, 0, 0, in->width, in->height);
  BOX2_COPY(info->databox, info->viewbox);
  info->tilesize = in->tilesize;

  MipCloseInputFile(in);

  return 0;
}

} // namespace xxx
