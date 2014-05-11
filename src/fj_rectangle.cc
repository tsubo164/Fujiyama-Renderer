/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_rectangle.h"
#include <stdio.h>

namespace fj {

void RctPrint(const struct Rectangle *rect)
{
  printf("(%d, %d) (%d, %d)\n",
      rect->xmin,
      rect->ymin,
      rect->xmax,
      rect->ymax);
}

} // namespace xxx
