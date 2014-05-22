/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_texture.h"
#include "fj_string_function.h"
#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_multi_thread.h"
#include "fj_tex_coord.h"
#include "fj_memory.h"
#include "fj_mipmap.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_box.h"
#include <stdlib.h>

namespace fj {

static const struct Color4 NO_TEXTURE_COLOR(1, .63, .63, 1);

struct TextureCache {
  struct FrameBuffer *fb;
  struct MipInput *mip;
  int last_xtile;
  int last_ytile;
};

static struct TextureCache *new_cache(void)
{
  struct TextureCache *cache = FJ_MEM_ALLOC(struct TextureCache);

  if (cache == NULL) {
    return NULL;
  }

  cache->fb = NULL;
  cache->mip = NULL;
  cache->last_xtile = -1;
  cache->last_ytile = -1;

  return cache;
}

static void free_cache(struct TextureCache *cache)
{
  if (cache == NULL) {
    return;
  }

  FbFree(cache->fb);
  MipCloseInputFile(cache->mip);
  FJ_MEM_FREE(cache);
}

static void free_cache_list(struct TextureCache **cache_list, int cache_count)
{
  int i;

  if (cache_list == NULL) {
    return;
  }

  for (i = 0; i < cache_count; i++) {
    struct TextureCache *cache = cache_list[i];
    free_cache(cache);
  }

  FJ_MEM_FREE(cache_list);
}

static int cache_open_mipmap(struct TextureCache *cache, const char *filename)
{
  if (cache == NULL) {
    return -1;
  }

  cache->mip = MipOpenInputFile(filename);
  if (cache->mip == NULL) {
    return -1;
  }

  cache->fb = FbNew();
  if (cache->fb == NULL) {
    return -1;
  }

  if (MipReadHeader(cache->mip)) {
    return -1;
  }

  FbResize(cache->fb, cache->mip->tilesize, cache->mip->tilesize, cache->mip->nchannels);
  return 0;
}

static void lookup_cache(struct TextureCache *cache, float u, float v, struct Color4 *rgba)
{
  struct TexCoord tex_space;
  struct TexCoord tile_space;

  int XNTILES, YNTILES;
  int xpxl, ypxl;
  int xtile, ytile;

  if (cache == NULL || cache->mip == NULL) {
    *rgba = NO_TEXTURE_COLOR;
    return;
  }

  tex_space.u = u - floor(u);
  tex_space.v = v - floor(v);

  XNTILES = cache->mip->xntiles;
  YNTILES = cache->mip->yntiles;

  tile_space.u = tex_space.u * XNTILES;
  tile_space.v = (1-tex_space.v) * YNTILES;

  xtile = (int) floor(tile_space.u);
  ytile = (int) floor(tile_space.v);

  if (xtile != cache->last_xtile || ytile != cache->last_ytile) {
    cache->mip->data = FbGetWritable(cache->fb, 0, 0, 0);
    MipReadTile(cache->mip, xtile, ytile);
    cache->last_xtile = xtile;
    cache->last_ytile = ytile;
  }

  xpxl = (int)( (tile_space.u - floor(tile_space.u)) * 64);
  ypxl = (int)( (tile_space.v - floor(tile_space.v)) * 64);

  FbGetColor(cache->fb, xpxl, ypxl, rgba);
}

struct Texture {
  char *filename;
  struct TextureCache **cache_list;
  int cache_count;
};

struct Texture *TexNew(void)
{
  struct Texture *tex = FJ_MEM_ALLOC(struct Texture);

  if (tex == NULL) {
    return NULL;
  }

  tex->filename = NULL;
  tex->cache_list = NULL;
  tex->cache_count = MtGetMaxThreadCount();

  return tex;
}

void TexFree(struct Texture *tex)
{
  if (tex == NULL) {
    return;
  }

  free_cache_list(tex->cache_list, tex->cache_count);
  StrFree(tex->filename);

  FJ_MEM_FREE(tex);
}

void TexLookup(const struct Texture *tex, float u, float v, struct Color4 *rgba)
{
  const int thread_id = MtGetThreadID();

  if (tex->cache_list[thread_id] == NULL) {
    struct TextureCache *cache = new_cache();
    cache_open_mipmap(cache, tex->filename);
    tex->cache_list[thread_id] = cache;
  }

  lookup_cache(tex->cache_list[thread_id], u, v, rgba);
}

int TexLoadFile(struct Texture *tex, const char *filename)
{
  int i;

  if (tex->filename != NULL) {
    free_cache_list(tex->cache_list, tex->cache_count);
    StrFree(tex->filename);
  }
  tex->filename = StrDup(filename);

  if (tex->cache_list != NULL) {
    free_cache_list(tex->cache_list, tex->cache_count);
  }

  tex->cache_list = FJ_MEM_ALLOC_ARRAY(struct TextureCache *, tex->cache_count);
  if (tex->cache_list == NULL) {
    /* TODO error handling */
    return -1;
  }

  for (i = 0; i < tex->cache_count; i++) {
    tex->cache_list[i] = NULL;
  }

  tex->cache_list[0] = new_cache();
  return cache_open_mipmap(tex->cache_list[0], tex->filename);
}

int TexGetWidth(const struct Texture *tex)
{
  if (tex->cache_list[0] == NULL) {
    return 0;
  }
  return tex->cache_list[0]->mip->width;
}

int TexGetHeight(const struct Texture *tex)
{
  if (tex->cache_list[0] == NULL) {
    return 0;
  }
  return tex->cache_list[0]->mip->height;
}

} // namespace xxx
