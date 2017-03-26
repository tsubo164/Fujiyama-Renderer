// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_rectangle.h"

namespace fj {

Int2 Rectangle::Size() const
{
  return max - min;
}

std::ostream &operator<<(std::ostream &os, const Rectangle &rect)
{
  return os << "(" << rect.min << ", " << rect.max << ")";
}

} // namespace xxx
