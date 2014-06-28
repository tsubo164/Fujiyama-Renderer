// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_POINT_CLOUDIO_H
#define FJ_POINT_CLOUDIO_H

#include "fj_compatibility.h"

namespace fj {

class Vector;

// point cloud input
class PtcInputFile;
FJ_API PtcInputFile *PtcOpenInputFile(const char *filename);
FJ_API void PtcCloseInputFile(PtcInputFile *in);

FJ_API void PtcReadHeader(PtcInputFile *in);
FJ_API void PtcReadData(PtcInputFile *in);

FJ_API int PtcGetInputPointCount(const PtcInputFile *in);
FJ_API void PtcSetInputPosition(PtcInputFile *in, Vector *P);
FJ_API void PtcSetInputAttributeDouble(PtcInputFile *in,
    const char *attr_name, double *attr_data);
FJ_API void PtcSetInputAttributeVector3(PtcInputFile *in,
    const char *attr_name, Vector *attr_data);

FJ_API int PtcGetInputAttributeCount(const PtcInputFile *in);

// point cloud output
class PtcOutputFile;
FJ_API PtcOutputFile *PtcOpenOutputFile(const char *filename);
FJ_API void PtcCloseOutputFile(PtcOutputFile *out);

FJ_API void PtcSetOutputPosition(PtcOutputFile *out, 
    const Vector *P, int point_count);

FJ_API void PtcSetOutputAttributeDouble(PtcOutputFile *out, 
    const char *attr_name, const double *attr_data);
FJ_API void PtcSetOutputAttributeVector3(PtcOutputFile *out, 
    const char *attr_name, const Vector *attr_data);

FJ_API void PtcWriteFile(PtcOutputFile *out);


// high level interface for loading mesh file
class PointCloud;
FJ_API int PtcLoadFile(PointCloud *ptc, const char *filename);

} // namespace xxx

#endif // FJ_XXX_H
