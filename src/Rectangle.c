/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Rectangle.h"
#include <stdio.h>

void RctPrint(const struct Rectangle *rect)
{
  printf("(%d, %d) (%d, %d)\n",
      rect->xmin,
      rect->ymin,
      rect->xmax,
      rect->ymax);
}
