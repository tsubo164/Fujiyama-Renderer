/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Texture.h"
#include "FrameBufferIO.h"
#include "FrameBuffer.h"
#include "TexCoord.h"
#include "Memory.h"
#include "Mipmap.h"
#include "Vector.h"
#include "Color.h"
#include "Box.h"
#include <stdlib.h>

static const struct Color4 NO_TEXTURE_COLOR = {1, .63, .63};

struct Texture {
  struct FrameBuffer *fb;
  int width;
  int height;

  struct MipInput *mip;
  int last_xtile;
  int last_ytile;
};

struct Texture *TexNew(void)
{
  struct Texture *tex = SI_MEM_ALLOC(struct Texture);

  if (tex == NULL)
    return NULL;

  tex->mip = NULL;
  tex->last_xtile = -1;
  tex->last_ytile = -1;

  return tex;
}

void TexFree(struct Texture *tex)
{
  if (tex == NULL)
    return;

  FbFree(tex->fb);
  MipCloseInputFile(tex->mip);
  SI_MEM_FREE(tex);
}

void TexLookup(struct Texture *tex, float u, float v, struct Color4 *rgba)
{
  struct TexCoord tex_space = {0, 0};
  struct TexCoord tile_space = {0, 0};

  int XNTILES, YNTILES;
  int xpxl, ypxl;
  int xtile, ytile;

  if (tex == NULL || tex->mip == NULL) {
    *rgba = NO_TEXTURE_COLOR;
    return;
  }

  tex_space.u = u - floor(u);
  tex_space.v = v - floor(v);

  XNTILES = tex->mip->xntiles;
  YNTILES = tex->mip->yntiles;

  tile_space.u = tex_space.u * XNTILES;
  tile_space.v = (1-tex_space.v) * YNTILES;

  xtile = (int) floor(tile_space.u);
  ytile = (int) floor(tile_space.v);

  if (xtile != tex->last_xtile || ytile != tex->last_ytile) {
    tex->mip->data = FbGetWritable(tex->fb, 0, 0, 0);
    MipReadTile(tex->mip, xtile, ytile);
    tex->last_xtile = xtile;
    tex->last_ytile = ytile;
  }

  xpxl = (int)( (tile_space.u - floor(tile_space.u)) * 64);
  ypxl = (int)( (tile_space.v - floor(tile_space.v)) * 64);

  FbGetColor(tex->fb, xpxl, ypxl, rgba);
}

int TexLoadFile(struct Texture *tex, const char *filename)
{
  tex->mip = MipOpenInputFile(filename);
  if (tex->mip == NULL)
    return -1;

  tex->fb = FbNew();
  if (tex->fb == NULL)
    return -1;

  if (MipReadHeader(tex->mip))
    return -1;

  tex->width = tex->mip->width;
  tex->height = tex->mip->height;

  FbResize(tex->fb, tex->mip->tilesize, tex->mip->tilesize, tex->mip->nchannels);

  return 0;
}

void TexGetResolution(const struct Texture *tex, int *xres, int *yres)
{
  if (tex->mip == NULL) {
    *xres = 0;
    *yres = 0;
  }

  *xres = tex->mip->width;
  *yres = tex->mip->height;
}

