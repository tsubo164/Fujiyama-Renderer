// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_POINT_CLOUDIO_H
#define FJ_POINT_CLOUDIO_H

#include "fj_compatibility.h"

namespace fj {

class PointCloud;
FJ_API int PtcSaveFile(const PointCloud &ptc, const char *filename);
FJ_API int PtcLoadFile(PointCloud &ptc, const char *filename);

} // namespace xxx

#endif // FJ_XXX_H
