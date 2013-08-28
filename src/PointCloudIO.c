/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "PointCloudIO.h"
#include "PointCloud.h"
#include "Memory.h"
#include "Vector.h"
#include "IO.h"

#define PTC_FILE_VERSION 1
#define PTC_FILE_MAGIC "PTCD"

struct PtcInputFile {
  struct InputFile *file;

  int point_count;
};

struct PtcInputFile *PtcOpenInputFile(const char *filename)
{
  struct PtcInputFile *in = MEM_ALLOC(struct PtcInputFile);

  if (in == NULL) {
    return NULL;
  }

  in->file = IOOpenInputFile(filename, PTC_FILE_MAGIC);
  if (in->file == NULL) {
    return NULL;
  }

  in->point_count = 0;
  IOSetInputInt(in->file, "point_count", &in->point_count, 1);
  IOEndInputHeader(in->file);

  return in;
}

void PtcCloseInputFile(struct PtcInputFile *in)
{
  if (in == NULL) {
    return;
  }

  IOCloseInputFile(in->file);
  MEM_FREE(in);
}

void PtcReadHeader(struct PtcInputFile *in)
{
  IOReadInputHeader(in->file);
}

void PtcReadData(struct PtcInputFile *in)
{
  IOReadInputData(in->file);
}

int PtcGetInputPointCount(const struct PtcInputFile *in)
{
  return in->point_count;
}

void PtcSetInputPosition(struct PtcInputFile *in, struct Vector *P)
{
  IOSetInputVector3(in->file, "P", P, PtcGetInputPointCount(in));
}

void PtcSetInputAttributeDouble(struct PtcInputFile *in,
    const char *attr_name, double *attr_data)
{
  IOSetInputDouble(in->file, attr_name, attr_data, PtcGetInputPointCount(in));
}

int PtcGetInputAttributeCount(const struct PtcInputFile *in)
{
  return IOGetInputDataChunkCount(in->file);
}

struct PtcOutputFile {
  struct OutputFile *file;

  int point_count;
};

struct PtcOutputFile *PtcOpenOutputFile(const char *filename)
{
  struct PtcOutputFile *out = MEM_ALLOC(struct PtcOutputFile);

  if (out == NULL) {
    return NULL;
  }

  out->file = IOOpenOutputFile(filename, PTC_FILE_MAGIC, PTC_FILE_VERSION);
  if (out->file == NULL) {
    return NULL;
  }

  out->point_count = 0;
  IOSetOutputInt(out->file, "point_count", &out->point_count, 1);
  IOEndOutputHeader(out->file);

  return out;
}

void PtcCloseOutputFile(struct PtcOutputFile *out)
{
  if (out == NULL) {
    return;
  }

  IOCloseOutputFile(out->file);
  MEM_FREE(out);
}

void PtcSetOutputPosition(struct PtcOutputFile *out, 
    const struct Vector *P, int point_count)
{
  out->point_count = point_count;
  IOSetOutputVector3 (out->file, "P", P, out->point_count);
}

void PtcSetOutputAttributeDouble(struct PtcOutputFile *out, 
    const char *attr_name, const double *attr_data)
{
  IOSetOutputDouble(out->file, attr_name, attr_data, out->point_count);
}

void PtcWriteFile(struct PtcOutputFile *out)
{
  IOWriteOutputHeader(out->file);
  IOWriteOutputData(out->file);
}

int PtcLoadFile(struct PointCloud *ptc, const char *filename)
{
  struct PtcInputFile *in = PtcOpenInputFile(filename);
  struct Vector *P = NULL;
  double *radius = NULL;
  int point_count = 0;
  int i;

  if (in == NULL) {
    return -1;
  }

  /* read file */
  PtcReadHeader(in);
  point_count = PtcGetInputPointCount(in);
  P      = MEM_ALLOC_ARRAY(struct Vector, point_count);
  radius = MEM_ALLOC_ARRAY(double,        point_count);

  PtcSetInputPosition(in, P);
  PtcSetInputAttributeDouble(in, "radius", radius);
  /*
  {
    const int N = PtcGetInputAttributeCount(in);
    printf("================== %d\n", N);
    int j;
    for (j = 0; j < N; j++) {
    }
  }
  */

  PtcReadData(in);

  PtcCloseInputFile(in);

  /* copy data to ptc */
  PtcAllocatePoint(ptc, point_count);

  for (i = 0; i < point_count; i++) {
    PtcSetPosition(ptc, i, &P[i]);
  }

  PtcComputeBounds(ptc);

  /* clean up */
  MEM_FREE(P);
  MEM_FREE(radius);

  return 0;
}

