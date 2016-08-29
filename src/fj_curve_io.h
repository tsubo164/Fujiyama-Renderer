// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_CURVEIO_H
#define FJ_CURVEIO_H

#include "fj_compatibility.h"
#include <vector>
#include <string>
#include <cstdio>

namespace fj {

class Curve;
class TexCoord;
class Vector;
class Color;

class FJ_API CurveInput {
public:
  CurveInput() {}
  ~CurveInput() {}

  FILE *file;
  int version;
  int nverts;
  int nvert_attrs;
  int ncurves;
  int ncurve_attrs;

  Vector *P;
  double *width;
  Color *Cd;
  TexCoord *uv;
  Vector *velocity;
  int *indices;

  std::vector<std::string> attr_names;

  std::vector<char> data_buffer;
};

class FJ_API CurveOutput {
public:
  CurveOutput() {}
  ~CurveOutput() {}

  FILE *file;
  int version;
  int nverts;
  int nvert_attrs;
  int ncurves;
  int ncurve_attrs;

  Vector *P;
  double *width;
  Color *Cd;
  TexCoord *uv;
  Vector *velocity;
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

// error no interfaces
FJ_API int CrvGetErrorNo(void);
FJ_API const char *CrvGetErrorMessage(int err);

// curve input file interfaces
FJ_API CurveInput *CrvOpenInputFile(const char *filename);
FJ_API void CrvCloseInputFile(CurveInput *in);
FJ_API int CrvReadHeader(CurveInput *in);
FJ_API int CrvReadAttribute(CurveInput *in);

// curve output file interfaces
FJ_API CurveOutput *CrvOpenOutputFile(const char *filename);
FJ_API void CrvCloseOutputFile(CurveOutput *out);
FJ_API void CrvWriteFile(CurveOutput *out);

// high level interface for loading curve file
FJ_API int CrvLoadFile(Curve *curve, const char *filename);

} // namespace xxx

#endif // FJ_XXX_H
