/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_TEXCOORD_H
#define FJ_TEXCOORD_H

#include "fj_compatibility.h"

namespace fj {

struct FJ_API TexCoord {
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

FJ_API struct TexCoord *TexCoordAlloc(long count);
FJ_API struct TexCoord *TexCoordRealloc(struct TexCoord *texcoord, long count);
FJ_API void TexCoordFree(struct TexCoord *texcoord);

} // namespace xxx

#endif /* FJ_XXX_H */
