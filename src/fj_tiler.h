/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_TILER_H
#define FJ_TILER_H

namespace fj {

struct Tile {
  int id;
  int xmin, ymin, xmax, ymax;
};

struct Tiler;
struct Rectangle;

extern struct Tiler *TlrNew(int xres, int yres, int xtile_size, int ytile_size);
extern void TlrFree(struct Tiler *tiler);

extern int TlrGetTileCount(const struct Tiler *tiler);
extern struct Tile *TlrGetTile(const struct Tiler *tiler, int index);

extern int TlrGenerateTiles(struct Tiler *tiler, const struct Rectangle *region);

} // namespace xxx

#endif /* FJ_XXX_H */
