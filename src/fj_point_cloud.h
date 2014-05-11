/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_POINTCLOUD_H
#define FJ_POINTCLOUD_H

namespace fj {

struct PointCloud;
struct PrimitiveSet;
struct Vector;

extern struct PointCloud *PtcNew(void);
extern void PtcFree(struct PointCloud *ptc);

extern struct Vector *PtcAllocatePoint(struct PointCloud *ptc, int point_count);
extern void PtcSetPosition(struct PointCloud *ptc, int index, const struct Vector *P);
extern void PtcGetPosition(const struct PointCloud *ptc, int index, struct Vector *P);

extern double *PtcAddAttributeDouble(struct PointCloud *ptc, const char *name);
extern struct Vector *PtcAddAttributeVector(struct PointCloud *ptc, const char *name);

extern void PtcComputeBounds(struct PointCloud *ptc);

extern void PtcGetPrimitiveSet(const struct PointCloud *ptc, struct PrimitiveSet *primset);

} // namespace xxx

#endif /* FJ_XXX_H */
