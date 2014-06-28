// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_RECTANGLE_H
#define FJ_RECTANGLE_H

#include "fj_compatibility.h"

namespace fj {

class FJ_API Rectangle {
public:
  Rectangle() : xmin(0), ymin(0), xmax(0), ymax(0) {}
  ~Rectangle() {}

  int xmin, ymin, xmax, ymax;
};

inline int SizeX(const Rectangle &rect)
{
  return rect.xmax - rect.xmin;
}

inline int SizeY(const Rectangle &rect)
{
  return rect.ymax - rect.ymin;
}

} // namespace xxx

#endif // FJ_XXX_H
