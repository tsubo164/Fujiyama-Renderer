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
#include "fj_mipmap.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_box.h"

#include <cstddef>

namespace fj {

static const struct Color4 NO_TEXTURE_COLOR(1, .63, .63, 1);

TextureCache::TextureCache() :
  fb_(NULL),
  mip_(NULL),
  last_xtile_(-1),
  last_ytile_(-1),
  is_open_(false)
{
}

TextureCache::~TextureCache()
{
  FbFree(fb_);
  MipCloseInputFile(mip_);
}

int TextureCache::OpenMipmap(const std::string &filename)
{
  if (filename == "") {
    return -1;
  }

  mip_ = MipOpenInputFile(filename.c_str());
  if (mip_ == NULL) {
    return -1;
  }

  fb_ = FbNew();
  if (fb_ == NULL) {
    return -1;
  }

  if (MipReadHeader(mip_)) {
    return -1;
  }

  FbResize(fb_, mip_->tilesize, mip_->tilesize, mip_->nchannels);
  is_open_ = true;

  return 0;
}

Color4 TextureCache::LookupTexture(float u, float v)
{
  if (mip_ == NULL) {
    return NO_TEXTURE_COLOR;
  }

  const TexCoord tex_space(
      u - floor(u),
      v - floor(v));

  const TexCoord tile_space(
           tex_space.u  * mip_->xntiles,
      (1 - tex_space.v) * mip_->yntiles);

  const int xtile = static_cast<int>(floor(tile_space.u));
  const int ytile = static_cast<int>(floor(tile_space.v));

  if (xtile != last_xtile_ || ytile != last_ytile_) {
    mip_->data = FbGetWritable(fb_, 0, 0, 0);
    MipReadTile(mip_, xtile, ytile);
    last_xtile_ = xtile;
    last_ytile_ = ytile;
  }

  const int xpxl = (int)( (tile_space.u - floor(tile_space.u)) * 64);
  const int ypxl = (int)( (tile_space.v - floor(tile_space.v)) * 64);

  // TODO FbGetColor returns Color4
  Color4 rgba;
  FbGetColor(fb_, xpxl, ypxl, &rgba);
  return rgba;
}

int TextureCache::GetTextureWidth() const
{
  if (mip_ == NULL)
    return 0;
  else
    return mip_->width;
}

int TextureCache::GetTextureHeight() const
{
  if (mip_ == NULL)
    return 0;
  else
    return mip_->height;
}

bool TextureCache::IsOpen() const
{
  return is_open_;
}

Texture::Texture() :
  filename(""),
  cache_list(MtGetMaxThreadCount()),
  cache_count(MtGetMaxThreadCount())
{
}

Texture::~Texture()
{
  //StrFree(filename);
}

struct Texture *TexNew(void)
{
  return new Texture();
}

void TexFree(struct Texture *tex)
{
  delete tex;
}

void TexLookup(const struct Texture *tex, float u, float v, struct Color4 *rgba)
{
  const int thread_id = MtGetThreadID();
  TextureCache &this_cache = const_cast<TextureCache&>(tex->cache_list[thread_id]);

  if (!this_cache.IsOpen()) {
    this_cache.OpenMipmap(tex->filename);
  }

  *rgba = this_cache.LookupTexture(u, v);
}

int TexLoadFile(struct Texture *tex, const char *filename)
{
  if (filename == NULL || std::string("") == filename) {
    return -1;
  }

  if (tex->filename == "") {
    const size_t cache_count = tex->cache_list.size();
    tex->cache_list.clear();
    tex->cache_list.resize(cache_count);
  }
  tex->filename = filename;

  return tex->cache_list[0].OpenMipmap(tex->filename);
}

int TexGetWidth(const struct Texture *tex)
{
  if (!tex->cache_list[0].IsOpen()) {
    return 0;
  }
  return tex->cache_list[0].GetTextureWidth();
}

int TexGetHeight(const struct Texture *tex)
{
  if (!tex->cache_list[0].IsOpen()) {
    return 0;
  }
  return tex->cache_list[0].GetTextureHeight();
}

} // namespace xxx
