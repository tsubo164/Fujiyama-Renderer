/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_POINTCLOUD_H
#define FJ_POINTCLOUD_H

#include "fj_compatibility.h"
#include "fj_types.h"

namespace fj {

struct PointCloud;
struct PrimitiveSet;
struct Vector;

FJ_API PointCloud *PtcNew(void);
FJ_API void PtcFree(PointCloud *ptc);

FJ_API Vector *PtcAllocatePoint(PointCloud *ptc, int point_count);
FJ_API void PtcSetPosition(PointCloud *ptc, int index, const Vector *P);
FJ_API void PtcGetPosition(const PointCloud *ptc, int index, Vector *P);

FJ_API Real *PtcAddAttributeDouble(PointCloud *ptc, const char *name);
FJ_API Vector *PtcAddAttributeVector(PointCloud *ptc, const char *name);

FJ_API void PtcComputeBounds(PointCloud *ptc);

FJ_API void PtcGetPrimitiveSet(const PointCloud *ptc, PrimitiveSet *primset);

} // namespace xxx

#endif /* FJ_XXX_H */
