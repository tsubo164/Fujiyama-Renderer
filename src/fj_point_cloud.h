/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
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

extern void PtcAllocatePoint(struct PointCloud *ptc, int point_count);
extern void PtcSetPosition(struct PointCloud *ptc, int index, const struct Vector *P);
extern void PtcGetPosition(const struct PointCloud *ptc, int index, struct Vector *P);

extern void PtcComputeBounds(struct PointCloud *ptc);

extern void PtcGetPrimitiveSet(const struct PointCloud *ptc, struct PrimitiveSet *primset);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
