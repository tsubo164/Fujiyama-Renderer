/*
Copyright (c) 2011-2015 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_rectangle.h"
#include <cstdio>

namespace fj {

void RctPrint(const Rectangle &rect)
{
  printf("(%d, %d) (%d, %d)\n",
      rect.xmin,
      rect.ymin,
      rect.xmax,
      rect.ymax);
}

} // namespace xxx
