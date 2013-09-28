/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_tiler.h"
#include "fj_rectangle.h"
#include "fj_numeric.h"
#include "fj_memory.h"

#include <math.h>
#include <assert.h>

struct Tiler {
  int total_ntiles;
  int xntiles, yntiles;
  struct Tile *tiles;

  int xres, yres;
  int xtile_size, ytile_size;
};

struct Tiler *TlrNew(int xres, int yres, int xtile_size, int ytile_size)
{
  struct Tiler *tiler = SI_MEM_ALLOC(struct Tiler);

  if (tiler == NULL)
    return NULL;

  tiler->total_ntiles = 0;
  tiler->xntiles = 0;
  tiler->yntiles = 0;
  tiler->tiles = NULL;

  assert(xres > 0);
  assert(yres > 0);
  assert(xtile_size > 0);
  assert(ytile_size > 0);

  tiler->xres = xres;
  tiler->yres = yres;
  tiler->xtile_size = xtile_size;
  tiler->ytile_size = ytile_size;

  return tiler;
}

void TlrFree(struct Tiler *tiler)
{
  if (tiler == NULL)
    return;

  if (tiler->tiles != NULL)
    SI_MEM_FREE(tiler->tiles);

  SI_MEM_FREE(tiler);
}

int TlrGetTileCount(const struct Tiler *tiler)
{
  return tiler->total_ntiles;
}

struct Tile *TlrGetTile(const struct Tiler *tiler, int index)
{
  if (index < 0 || index >= TlrGetTileCount(tiler)) {
    return NULL;
  }
  return &tiler->tiles[index];
}

int TlrGenerateTiles(struct Tiler *tiler, const struct Rectangle *region)
{
  int id;
  int x, y;
  struct Tile *tiles = NULL, *tile = NULL;

  const int xres = tiler->xres;
  const int yres = tiler->yres;
  const int xtile_size = tiler->xtile_size;
  const int ytile_size = tiler->ytile_size;

  const int xmin = region->xmin;
  const int ymin = region->ymin;
  const int xmax = region->xmax;
  const int ymax = region->ymax;

  const int XMIN = (int) floor(MAX(0, xmin) / (double) xtile_size);
  const int YMIN = (int) floor(MAX(0, ymin) / (double) ytile_size);
  const int XMAX = (int) ceil(MIN(xres, xmax) / (double) xtile_size);
  const int YMAX = (int) ceil(MIN(yres, ymax) / (double) ytile_size);

  const int xntiles = XMAX - XMIN;
  const int yntiles = YMAX - YMIN;
  const int total_ntiles = xntiles * yntiles;

  assert(xmin < xmax);
  assert(ymin < ymax);

  if (tiler->tiles != NULL) {
    SI_MEM_FREE(tiler->tiles);
  }

  tiles = SI_MEM_ALLOC_ARRAY(struct Tile, total_ntiles);
  if (tiles == NULL) {
    return -1;
  }

  tile = tiles;
  id = 0;
  for (y = YMIN; y < YMAX; y++) {
    for (x = XMIN; x < XMAX; x++) {
      tile->id = id;
      tile->xmin = x * xtile_size;
      tile->ymin = y * ytile_size;

      tile->xmin = MAX(tile->xmin, xmin);
      tile->ymin = MAX(tile->ymin, ymin);

      tile->xmax = (x + 1) * xtile_size;
      tile->ymax = (y + 1) * ytile_size;

      tile->xmax = MIN(tile->xmax, xmax);
      tile->ymax = MIN(tile->ymax, ymax);
      tile++;
      id++;
    }
  }

  tiler->total_ntiles = total_ntiles;
  tiler->xntiles = xntiles;
  tiler->yntiles = yntiles;
  tiler->tiles = tiles;

  return 0;
}
