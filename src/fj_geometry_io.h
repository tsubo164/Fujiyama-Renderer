// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_GEOMETRY_IO_H
#define FJ_GEOMETRY_IO_H

#include "fj_geometry.h"
#include <fstream>
#include <string>

namespace fj {

//int WriteGeometry(const std::string &filename, const Geometry &geo);

class GeoOutputFile {
public:
  GeoOutputFile(const std::string &filename);
  virtual ~GeoOutputFile();

  int Write(const Geometry &geo);
private:
  GeoOutputFile(const GeoOutputFile &);
  const GeoOutputFile &operator=(const GeoOutputFile &);

  std::ofstream file_;
};

class GeoInputFile {
public:
  GeoInputFile(const std::string &filename);
  virtual ~GeoInputFile();

  int Read(Geometry &geo);
private:
  GeoInputFile(const GeoInputFile &);
  const GeoInputFile &operator=(const GeoInputFile &);

  std::ifstream file_;
};

} // namespace xxx

#endif // FJ_XXX_H
