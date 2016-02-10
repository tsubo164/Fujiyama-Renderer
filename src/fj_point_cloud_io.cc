// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_point_cloud_io.h"
#include "fj_geometry_io.h"
#include "fj_point_cloud.h"

namespace fj {

int PtcSaveFile(const PointCloud &ptc, const char *filename)
{
  GeoOutputFile f(filename);
  f.Write(ptc);

  return 0;
}

int PtcLoadFile(PointCloud &ptc, const char *filename)
{
  GeoInputFile f(filename);
  f.Read(ptc);

  return 0;
}

} // namespace xxx
