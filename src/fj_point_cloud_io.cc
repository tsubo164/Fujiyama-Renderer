/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_point_cloud_io.h"
#include "fj_point_cloud.h"
#include "fj_memory.h"
#include "fj_vector.h"
#include "fj_io.h"

#include <vector>

#define PTC_FILE_VERSION 1
#define PTC_FILE_MAGIC "PTCD"

namespace fj {

struct PtcInputFile {
  struct InputFile *file;

  int point_count;
};

struct PtcInputFile *PtcOpenInputFile(const char *filename)
{
  struct PtcInputFile *in = FJ_MEM_ALLOC(struct PtcInputFile);

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
  FJ_MEM_FREE(in);
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

void PtcSetInputAttributeVector3(struct PtcInputFile *in,
    const char *attr_name, struct Vector *attr_data)
{
  IOSetInputVector3(in->file, attr_name, attr_data, PtcGetInputPointCount(in));
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
  struct PtcOutputFile *out = FJ_MEM_ALLOC(struct PtcOutputFile);

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
  FJ_MEM_FREE(out);
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

void PtcSetOutputAttributeVector3(struct PtcOutputFile *out, 
    const char *attr_name, const struct Vector *attr_data)
{
  IOSetOutputVector3(out->file, attr_name, attr_data, out->point_count);
}

void PtcWriteFile(struct PtcOutputFile *out)
{
  IOWriteOutputHeader(out->file);
  IOWriteOutputData(out->file);
}

int PtcLoadFile(struct PointCloud *ptc, const char *filename)
{
  struct PtcInputFile *in = PtcOpenInputFile(filename);

  if (in == NULL) {
    return -1;
  }

  /* read file */
  PtcReadHeader(in);
  const int point_count = PtcGetInputPointCount(in);

  std::vector<Vector> P(point_count);
  std::vector<Vector> velocity(point_count);
  std::vector<double> radius(point_count);

  PtcSetInputPosition(in, &P[0]);
  PtcSetInputAttributeVector3(in, "velocity", &velocity[0]);
  PtcSetInputAttributeDouble(in, "radius", &radius[0]);

  PtcReadData(in);

  PtcCloseInputFile(in);

  /* copy data to ptc */
  ptc->SetPointCount(point_count);
  ptc->AddPointPosition();
  ptc->AddPointVelocity();
  ptc->AddPointRadius();

  for (int i = 0; i < ptc->GetPointCount(); i++) {
    ptc->SetPointPosition(i, P[i]);
    ptc->SetPointVelocity(i, velocity[i]);
    ptc->SetPointRadius(i, radius[i]);
  }

  ptc->ComputeBounds();

  return 0;
}

} // namespace fj
