/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef CALLBACK_H
#define CALLBACK_H

#include "Rectangle.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WorkInfo {
  int worker_id;
  int region_id;
  int total_region_count;
  int total_sample_count;
  struct Rectangle region;
};

typedef void (*WorkStartCallback)(void *data, const struct WorkInfo *info);
typedef int (*WorkIncrementCallback)(void *data);
typedef void (*WorkDoneCallback)(void *data, const struct WorkInfo *info);

extern void SetupWorkInfo(struct WorkInfo *info,
    int worker_id,
    int region_id,
    int total_region_count,
    int total_sample_count,
    const struct Rectangle *region);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */
