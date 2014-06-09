/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_TEXTURE_H
#define FJ_TEXTURE_H

#include <string>
#include <vector>

namespace fj {

struct Texture;
struct Color4;

// Texture cache for each thread
class TextureCache {
public:
  TextureCache();
  ~TextureCache();

  int OpenMipmap(const std::string &filename);
  Color4 LookupTexture(float u, float v);

  int GetTextureWidth() const;
  int GetTextureHeight() const;
  bool IsOpen() const;

private:
  struct FrameBuffer *fb_;
  struct MipInput *mip_;
  int last_xtile_;
  int last_ytile_;
  bool is_open_;
};

class Texture {
public:
  Texture();
  ~Texture();

public:
  std::string filename;
  std::vector<TextureCache> cache_list;
  int cache_count;
};

extern struct Texture *TexNew(void);
extern void TexFree(struct Texture *tex);

/*
 * Looks up a value of texture image.
 * (r, r, r, 1) will be returned when texture is grayscale.
 * (r, g, b, 1) will be returned when texture is rgb.
 * (r, g, b, a) will be returned when texture is rgba.
 */
extern void TexLookup(const struct Texture *tex, float u, float v, struct Color4 *rgba);
extern int TexLoadFile(struct Texture *tex, const char *filename);

extern int TexGetWidth(const struct Texture *tex);
extern int TexGetHeight(const struct Texture *tex);

} // namespace xxx

#endif // FJ_XXX_H
