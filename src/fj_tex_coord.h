/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_TEXCOORD_H
#define FJ_TEXCOORD_H

namespace fj {

struct TexCoord {
  float u, v;
};

extern struct TexCoord *TexCoordAlloc(long count);
extern struct TexCoord *TexCoordRealloc(struct TexCoord *texcoord, long count);
extern void TexCoordFree(struct TexCoord *texcoord);

} // namespace xxx

#endif /* FJ_XXX_H */
