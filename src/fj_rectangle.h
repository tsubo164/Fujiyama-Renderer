// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_RECTANGLE_H
#define FJ_RECTANGLE_H

#include "fj_compatibility.h"
#include "fj_vector.h"
#include <iostream>

namespace fj {

class FJ_API Rectangle {
public:
  Rectangle() : min(), max() {}
  ~Rectangle() {}

  Int2 Size() const;
  int SizeX() const;
  int SizeY() const;

public:
  Int2 min, max;
};

inline Int2 Rectangle::Size() const
{
  return max - min;
}

inline int Rectangle::SizeX() const
{
  return max[0] - min[0];
}

inline int Rectangle::SizeY() const
{
  return max[1] - min[1];
}

std::ostream &operator<<(std::ostream &os, const Rectangle &rect);

} // namespace xxx

#endif // FJ_XXX_H
