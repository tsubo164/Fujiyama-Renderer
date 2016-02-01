// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_point_cloud_io.h"
#include "fj_point_cloud.h"
#include "fj_vector.h"
#include "fj_io.h"

#include <vector>
#include <cstddef>

#define PTC_FILE_VERSION 1
#define PTC_FILE_MAGIC "PTCD"

//TODO TEST
#include "fj_geometry_io.h"

namespace fj {

class PtcInputFile {
public:
  PtcInputFile() {}
  ~PtcInputFile() {}

public:
  InputFile *file;

  int point_count;
};

PtcInputFile *PtcOpenInputFile(const char *filename)
{
  PtcInputFile *in = new PtcInputFile();
  if (in == NULL) {
    return NULL;
  }

  in->file = IOOpenInputFile(filename, PTC_FILE_MAGIC);
  if (in->file == NULL) {
    PtcCloseInputFile(in);
    return NULL;
  }

  in->point_count = 0;
  IOSetInputInt(in->file, "point_count", &in->point_count, 1);
  IOEndInputHeader(in->file);

  return in;
}

void PtcCloseInputFile(PtcInputFile *in)
{
  if (in == NULL) {
    return;
  }

  IOCloseInputFile(in->file);
  delete in;
}

void PtcReadHeader(PtcInputFile *in)
{
  IOReadInputHeader(in->file);
}

void PtcReadData(PtcInputFile *in)
{
  IOReadInputData(in->file);
}

int PtcGetInputPointCount(const PtcInputFile *in)
{
  return in->point_count;
}

void PtcSetInputPosition(PtcInputFile *in, Vector *P)
{
  IOSetInputVector3(in->file, "P", P, PtcGetInputPointCount(in));
}

void PtcSetInputAttributeDouble(PtcInputFile *in,
    const char *attr_name, double *attr_data)
{
  IOSetInputDouble(in->file, attr_name, attr_data, PtcGetInputPointCount(in));
}

void PtcSetInputAttributeVector3(PtcInputFile *in,
    const char *attr_name, Vector *attr_data)
{
  IOSetInputVector3(in->file, attr_name, attr_data, PtcGetInputPointCount(in));
}

int PtcGetInputAttributeCount(const PtcInputFile *in)
{
  return IOGetInputDataChunkCount(in->file);
}

class PtcOutputFile {
public:
  PtcOutputFile() {}
  ~PtcOutputFile() {}

public:
  OutputFile *file;

  int point_count;
};

PtcOutputFile *PtcOpenOutputFile(const char *filename)
{
  PtcOutputFile *out = new PtcOutputFile();
  if (out == NULL) {
    return NULL;
  }

  out->file = IOOpenOutputFile(filename, PTC_FILE_MAGIC, PTC_FILE_VERSION);
  if (out->file == NULL) {
    PtcCloseOutputFile(out);
    return NULL;
  }

  out->point_count = 0;
  IOSetOutputInt(out->file, "point_count", &out->point_count, 1);
  IOEndOutputHeader(out->file);

  return out;
}

void PtcCloseOutputFile(PtcOutputFile *out)
{
  if (out == NULL) {
    return;
  }

  IOCloseOutputFile(out->file);
  delete out;
}

void PtcSetOutputPosition(PtcOutputFile *out, 
    const Vector *P, int point_count)
{
  out->point_count = point_count;
  IOSetOutputVector3 (out->file, "P", P, out->point_count);
}

void PtcSetOutputAttributeDouble(PtcOutputFile *out, 
    const char *attr_name, const double *attr_data)
{
  IOSetOutputDouble(out->file, attr_name, attr_data, out->point_count);
}

void PtcSetOutputAttributeVector3(PtcOutputFile *out, 
    const char *attr_name, const Vector *attr_data)
{
  IOSetOutputVector3(out->file, attr_name, attr_data, out->point_count);
}

void PtcWriteFile(PtcOutputFile *out)
{
  IOWriteOutputHeader(out->file);
  IOWriteOutputData(out->file);
}

int PtcLoadFile(PointCloud *ptc, const char *filename)
{
  //TODO TEST
  {
    GeoInputFile geofile(filename);
    geofile.Read(*ptc);
    ptc->ComputeBounds();
  }
  return 0;
#if 0

  PtcInputFile *in = PtcOpenInputFile(filename);

  if (in == NULL) {
    return -1;
  }

  // read file
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

  // copy data to ptc
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

  {
    GeoOutputFile f("../ptc.fjgeo");
    f.Write(*ptc);
  }
  {
    GeoInputFile f("../ptc.fjgeo");
    Geometry geo;
    f.Read(geo);
  }

  return 0;
#endif
}

} // namespace xxx
