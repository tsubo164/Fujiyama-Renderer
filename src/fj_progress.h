/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_PROGRESS_H
#define FJ_PROGRESS_H

#include "fj_compatibility.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t Iteration;
struct Progress;

extern struct Progress *PrgNew(void);
extern void PrgFree(struct Progress *progress);

extern void PrgStart(struct Progress *progress, Iteration total_iterations);
extern void PrgIncrement(struct Progress *progress);
extern void PrgDone(struct Progress *progress);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
