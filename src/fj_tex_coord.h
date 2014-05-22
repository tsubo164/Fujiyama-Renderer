/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_TEXCOORD_H
#define FJ_TEXCOORD_H

namespace fj {

struct TexCoord {
  TexCoord() : u(0), v(0) {}
  TexCoord(float uu, float vv) : u(uu), v(vv) {}
  ~TexCoord() {}

  const float &operator[](int i) const
  {
    switch(i) {
    case 0: return u;
    case 1: return v;
    default: return u; // TODO ERROR HANDLING
    }
  }
  float &operator[](int i)
  {
    switch(i) {
    case 0: return u;
    case 1: return v;
    default: return u; // TODO ERROR HANDLING
    }
  }

  float u, v;
};

extern struct TexCoord *TexCoordAlloc(long count);
extern struct TexCoord *TexCoordRealloc(struct TexCoord *texcoord, long count);
extern void TexCoordFree(struct TexCoord *texcoord);

} // namespace xxx

#endif /* FJ_XXX_H */
