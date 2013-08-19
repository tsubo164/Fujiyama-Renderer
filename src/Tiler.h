/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TILER_H
#define TILER_H

#ifdef __cplusplus
extern "C" {
#endif

struct Tiler;

struct Tile {
  int id;
  int xmin, ymin, xmax, ymax;
};

extern struct Tiler *TlrNew(int xres, int yres, int xtile_size, int ytile_size);
extern void TlrFree(struct Tiler *tiler);

extern int TlrGetTileCount(const struct Tiler *tiler);
extern struct Tile *TlrGetNextTile(struct Tiler *tiler);

/* TODO TEST */
extern struct Tile *TlrGetTile(const struct Tiler *tiler, int index);

extern int TlrGenerateTiles(struct Tiler *tiler, int xmin, int ymin, int xmax, int ymax);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */
