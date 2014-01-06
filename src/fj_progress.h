/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_PROGRESS_H
#define FJ_PROGRESS_H

#include "fj_compatibility.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t Iteration;

typedef int ProgressStatus;
enum {
  PROGRESS_ONGOING = 0,
  PROGRESS_DONE = 1
};

struct Progress {
  Iteration total_iterations;
  Iteration iteration;
};

extern void PrgStart(struct Progress *progress, Iteration total_iterations);
extern ProgressStatus PrgIncrement(struct Progress *progress);
extern void PrgDone(struct Progress *progress);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
