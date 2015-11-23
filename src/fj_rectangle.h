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

public:
  Int2 min, max;
};

inline Int2 Rectangle::Size() const
{
  return max - min;
}

std::ostream &operator<<(std::ostream &os, const Rectangle &rect);

} // namespace xxx

#endif // FJ_XXX_H
