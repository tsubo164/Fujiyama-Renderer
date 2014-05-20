/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_COLOR_H
#define FJ_COLOR_H

namespace fj {

struct Color {
  float r, g, b;
};

struct Color4 {
  float r, g, b, a;
};

extern struct Color *ColAlloc(long count);
extern struct Color *ColRealloc(struct Color *c, long count);
extern void ColFree(struct Color *c);

#define ColSet(dst,R,G,B) do { \
  (dst)->r = (R); \
  (dst)->g = (G); \
  (dst)->b = (B); \
  } while(0)

#define COL_LUMINANCE(c) (.298912*(c)->r+.586611*(c)->g+.114478*(c)->b)

inline void ColLerp(Color *dst, const Color &A, const Color &B, float t)
{
  dst->r = (1 - t) * A.r + t * B.r;
  dst->g = (1 - t) * A.g + t * B.g;
  dst->b = (1 - t) * A.b + t * B.b;
}

} // namespace xxx

#endif /* FJ_XXX_H */
