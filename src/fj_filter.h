/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_FILTER_H
#define FJ_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

struct Filter;

enum {
  FLT_BOX = 0,
  FLT_GAUSSIAN
};

extern struct Filter *FltNew(int filtertype, double xwidth, double ywidth);
extern void FltFree(struct Filter *filter);

extern double FltEvaluate(const struct Filter *filter, double x, double y);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */

