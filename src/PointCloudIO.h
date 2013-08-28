/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef POINTCLOUDIO_H
#define POINTCLOUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

struct Vector;

/* point cloud input */
struct PtcInputFile;
extern struct PtcInputFile *PtcOpenInputFile(const char *filename);
extern void PtcCloseInputFile(struct PtcInputFile *in);

extern void PtcReadHeader(struct PtcInputFile *in);
extern void PtcReadData(struct PtcInputFile *in);

extern int PtcGetInputPointCount(const struct PtcInputFile *in);
extern void PtcSetInputPosition(struct PtcInputFile *in, struct Vector *P);
extern void PtcSetInputAttributeDouble(struct PtcInputFile *in,
    const char *attr_name, double *attr_data);

extern int PtcGetInputAttributeCount(const struct PtcInputFile *in);

/* point cloud output */
struct PtcOutputFile;
extern struct PtcOutputFile *PtcOpenOutputFile(const char *filename);
extern void PtcCloseOutputFile(struct PtcOutputFile *out);

extern void PtcSetOutputPosition(struct PtcOutputFile *out, 
    const struct Vector *P, int point_count);

extern void PtcSetOutputAttributeDouble(struct PtcOutputFile *out, 
    const char *attr_name, const double *attr_data);

extern void PtcWriteFile(struct PtcOutputFile *out);


/* high level interface for loading mesh file */
struct PointCloud;
extern int PtcLoadFile(struct PointCloud *ptc, const char *filename);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

