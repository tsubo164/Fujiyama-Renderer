/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_CURVEIO_H
#define FJ_CURVEIO_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Curve;
struct TexCoord;
struct Vector;
struct Color;

struct CurveInput {
  FILE *file;
  int version;
  int nverts;
  int nvert_attrs;
  int ncurves;
  int ncurve_attrs;

  struct Vector *P;
  double *width;
  struct Color *Cd;
  struct TexCoord *uv;
  int *indices;

  char **attr_names;
};

struct CurveOutput {
  FILE *file;
  int version;
  int nverts;
  int nvert_attrs;
  int ncurves;
  int ncurve_attrs;

  struct Vector *P;
  double *width;
  struct Color *Cd;
  struct TexCoord *uv;
  int *indices;
};

enum CrvErrorNo {
  ERR_CRV_NOERR = 0,
  ERR_CRV_NOMEM,
  ERR_CRV_NOFILE,
  ERR_CRV_NOTMESH,
  ERR_CRV_BADVER,
  ERR_CRV_BADATTRNAME
};

/* error no interfaces */
extern int CrvGetErrorNo(void);
extern const char *CrvGetErrorMessage(int err);

/* curve input file interfaces */
extern struct CurveInput *CrvOpenInputFile(const char *filename);
extern void CrvCloseInputFile(struct CurveInput *in);
extern int CrvReadHeader(struct CurveInput *in);
extern int CrvReadAttribute(struct CurveInput *in, void *data);

/* curve output file interfaces */
extern struct CurveOutput *CrvOpenOutputFile(const char *filename);
extern void CrvCloseOutputFile(struct CurveOutput *out);
extern void CrvWriteFile(struct CurveOutput *out);

/* high level interface for loading curve file */
extern int CrvLoadFile(struct Curve *curve, const char *filename);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */

