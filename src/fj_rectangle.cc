// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_rectangle.h"

namespace fj {

std::ostream &operator<<(std::ostream &os, const Rectangle &rect)
{
  return os << "(" << rect.min << ", " << rect.max << ")";
}

} // namespace xxx
