/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FILTER_H
#define FILTER_H

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

#endif /* XXX_H */

