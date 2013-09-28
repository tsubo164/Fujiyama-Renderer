/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_RECTANGLE_H
#define FJ_RECTANGLE_H

#ifdef __cplusplus
extern "C" {
#endif

struct Rectangle {
  int xmin, ymin, xmax, ymax;
};

extern void RctPrint(const struct Rectangle *rect);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
