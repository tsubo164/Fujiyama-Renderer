/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_texture.h"
#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_multi_thread.h"
#include "fj_tex_coord.h"
#include "fj_mipmap.h"
#include "fj_vector.h"
#include "fj_color.h"

#include <cstddef>

namespace fj {

static const Color4 NO_TEXTURE_COLOR(1, .63, .63, 1);

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
    filename_(""),
    cache_list_(MtGetMaxThreadCount())
{
}

Texture::~Texture()
{
}

Color4 Texture::Lookup(float u, float v) const
{
  const int thread_id = MtGetThreadID();
  TextureCache &this_cache = const_cast<TextureCache&>(cache_list_[thread_id]);

  if (!this_cache.IsOpen()) {
    this_cache.OpenMipmap(filename_);
  }

  return this_cache.LookupTexture(u, v);
}

int Texture::LoadFile(const std::string &filename)
{
  if (filename_ == "") {
    const size_t cache_count = cache_list_.size();
    cache_list_.clear();
    cache_list_.resize(cache_count);
  }
  filename_ = filename;

  return cache_list_[0].OpenMipmap(filename_);
}

int Texture::GetWidth() const
{
  if (!cache_list_[0].IsOpen()) {
    return 0;
  }
  return cache_list_[0].GetTextureWidth();
}

int Texture::GetHeight() const
{
  if (!cache_list_[0].IsOpen()) {
    return 0;
  }
  return cache_list_[0].GetTextureHeight();
}

Texture *TexNew(void)
{
  return new Texture();
}

void TexFree(Texture *tex)
{
  delete tex;
}

} // namespace xxx
