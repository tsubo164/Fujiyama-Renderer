/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_FILTER_H
#define FJ_FILTER_H

namespace fj {

struct Filter;

enum {
  FLT_BOX = 0,
  FLT_GAUSSIAN
};

extern struct Filter *FltNew(int filtertype, double xwidth, double ywidth);
extern void FltFree(struct Filter *filter);

extern double FltEvaluate(const struct Filter *filter, double x, double y);

} // namespace xxx

#endif /* FJ_XXX_H */
