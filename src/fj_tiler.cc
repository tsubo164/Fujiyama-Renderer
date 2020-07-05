// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_tiler.h"
#include "fj_rectangle.h"
#include "fj_numeric.h"

#include <cstddef>
#include <cassert>

namespace fj {

Tiler::Tiler() :
  total_ntiles_(0),
  xntiles_(0),
  yntiles_(0),
  tiles_(),

  xres_(0),
  yres_(0),
  xtile_size_(0),
  ytile_size_(0)
{
}

Tiler::~Tiler()
{
}

int Tiler::GetTileCount() const
{
  return total_ntiles_;
}

const Tile *Tiler::GetTile(int index) const
{
  if (index < 0 || index >= GetTileCount()) {
    return NULL;
  }
  return &tiles_[index];
}

void Tiler::Divide(int xres, int yres, int xtile_size, int ytile_size)
{
  assert(xres > 0);
  assert(yres > 0);
  assert(xtile_size > 0);
  assert(ytile_size > 0);

  xres_ = xres;
  yres_ = yres;
  xtile_size_ = xtile_size;
  ytile_size_ = ytile_size;
}

void Tiler::GenerateTiles(const Rectangle &region)
{
  const int xres = xres_;
  const int yres = yres_;
  const int xtile_size = xtile_size_;
  const int ytile_size = ytile_size_;

  const int xmin = region.min[0];
  const int ymin = region.min[1];
  const int xmax = region.max[0];
  const int ymax = region.max[1];

  const int XMIN = (int) floor(Max(0, xmin) / (double) xtile_size);
  const int YMIN = (int) floor(Max(0, ymin) / (double) ytile_size);
  const int XMAX = (int) ceil(Min(xres, xmax) / (double) xtile_size);
  const int YMAX = (int) ceil(Min(yres, ymax) / (double) ytile_size);

  const int xntiles = XMAX - XMIN;
  const int yntiles = YMAX - YMIN;
  const int total_ntiles = xntiles * yntiles;

  assert(xmin < xmax);
  assert(ymin < ymax);

  std::vector<Tile> tmp_tiles(total_ntiles);
  if (tmp_tiles.empty()) {
    return;
  }

  Tile *tile = &tmp_tiles[0];
  int id = 0;

  for (int y = YMIN; y < YMAX; y++) {
    for (int x = XMIN; x < XMAX; x++) {
      tile->id = id;
      tile->xmin = x * xtile_size;
      tile->ymin = y * ytile_size;

      tile->xmin = Max(tile->xmin, xmin);
      tile->ymin = Max(tile->ymin, ymin);

      tile->xmax = (x + 1) * xtile_size;
      tile->ymax = (y + 1) * ytile_size;

      tile->xmax = Min(tile->xmax, xmax);
      tile->ymax = Min(tile->ymax, ymax);

      tile++;
      id++;
    }
  }

  // commit
  total_ntiles_ = total_ntiles;
  xntiles_ = xntiles;
  yntiles_ = yntiles;
  tiles_.swap(tmp_tiles);
}

} // namespace xxx
