/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#ifdef __cplusplus
extern "C" {
#endif

struct PointCloud;
struct PrimitiveSet;

extern struct PointCloud *PtcNew(void);
extern void PtcFree(struct PointCloud *ptc);

extern void PtcAllocatePoint(struct PointCloud *ptc, int npoints);

extern void PtcGetPrimitiveSet(const struct PointCloud *ptc, struct PrimitiveSet *primset);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

