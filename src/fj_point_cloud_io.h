// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_POINT_CLOUDIO_H
#define FJ_POINT_CLOUDIO_H

namespace fj {

class Vector;

// point cloud input
class PtcInputFile;
extern PtcInputFile *PtcOpenInputFile(const char *filename);
extern void PtcCloseInputFile(PtcInputFile *in);

extern void PtcReadHeader(PtcInputFile *in);
extern void PtcReadData(PtcInputFile *in);

extern int PtcGetInputPointCount(const PtcInputFile *in);
extern void PtcSetInputPosition(PtcInputFile *in, Vector *P);
extern void PtcSetInputAttributeDouble(PtcInputFile *in,
    const char *attr_name, double *attr_data);
extern void PtcSetInputAttributeVector3(PtcInputFile *in,
    const char *attr_name, Vector *attr_data);

extern int PtcGetInputAttributeCount(const PtcInputFile *in);

// point cloud output
class PtcOutputFile;
extern PtcOutputFile *PtcOpenOutputFile(const char *filename);
extern void PtcCloseOutputFile(PtcOutputFile *out);

extern void PtcSetOutputPosition(PtcOutputFile *out, 
    const Vector *P, int point_count);

extern void PtcSetOutputAttributeDouble(PtcOutputFile *out, 
    const char *attr_name, const double *attr_data);
extern void PtcSetOutputAttributeVector3(PtcOutputFile *out, 
    const char *attr_name, const Vector *attr_data);

extern void PtcWriteFile(PtcOutputFile *out);


// high level interface for loading mesh file
class PointCloud;
extern int PtcLoadFile(PointCloud *ptc, const char *filename);

} // namespace xxx

#endif // FJ_XXX_H
