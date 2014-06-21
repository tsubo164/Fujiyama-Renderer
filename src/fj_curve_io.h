// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_CURVEIO_H
#define FJ_CURVEIO_H

#include <vector>
#include <string>
#include <cstdio>

namespace fj {

class Curve;
class TexCoord;
class Vector;
class Color;

class CurveInput {
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

class CurveOutput {
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
extern int CrvGetErrorNo(void);
extern const char *CrvGetErrorMessage(int err);

// curve input file interfaces
extern CurveInput *CrvOpenInputFile(const char *filename);
extern void CrvCloseInputFile(CurveInput *in);
extern int CrvReadHeader(CurveInput *in);
extern int CrvReadAttribute(CurveInput *in);

// curve output file interfaces
extern CurveOutput *CrvOpenOutputFile(const char *filename);
extern void CrvCloseOutputFile(CurveOutput *out);
extern void CrvWriteFile(CurveOutput *out);

// high level interface for loading curve file
extern int CrvLoadFile(Curve *curve, const char *filename);

} // namespace xxx

#endif // FJ_XXX_H
