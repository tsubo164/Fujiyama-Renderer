/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_POINTCLOUD_H
#define FJ_POINTCLOUD_H

#ifdef __cplusplus
extern "C" {
#endif

struct PointCloud;
struct PrimitiveSet;
struct Vector;

extern struct PointCloud *PtcNew(void);
extern void PtcFree(struct PointCloud *ptc);

/* TODO TEST */
#include "fj_compatibility.h"
typedef int64_t PtcIndex;
extern void PtcSetPointCount(struct PointCloud *ptc, PtcIndex count);
extern PtcIndex PtcGetPointCount(const struct PointCloud *ptc);

extern void PtcAllocatePoint(struct PointCloud *ptc, int point_count);
extern void PtcSetPosition(struct PointCloud *ptc, const struct Vector *P);
extern void PtcGetPosition(const struct PointCloud *ptc, int index, struct Vector *P);

extern void PtcComputeBounds(struct PointCloud *ptc);

extern void PtcGetPrimitiveSet(const struct PointCloud *ptc, struct PrimitiveSet *primset);

/* TODO rename */
extern int PtcWriteFile2(const struct PointCloud *ptc, const char *filename);
extern int PtcReadFile(struct PointCloud *ptc, const char *filename);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
