// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_TILER_H
#define FJ_TILER_H

#include <vector>

namespace fj {

class Rectangle;

class Tile {
public:
  Tile() : id(0), xmin(0), ymin(0), xmax(0), ymax(0) {}
  ~Tile() {}
  int id;
  int xmin, ymin, xmax, ymax;
};

class Tiler {
public:
  Tiler();
  ~Tiler();

  int GetTileCount() const;
  const Tile *GetTile(int index) const;

  void Divide(int xres, int yres, int xtile_size, int ytile_size);
  void GenerateTiles(const Rectangle &region);

public:
  int total_ntiles_;
  int xntiles_;
  int yntiles_;
  std::vector<Tile> tiles_;

  int xres_;
  int yres_;
  int xtile_size_;
  int ytile_size_;
};

} // namespace xxx

#endif // FJ_XXX_H
