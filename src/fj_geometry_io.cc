// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_geometry_io.h"
#include "fj_geometry.h"
#include "fj_serialize.h"
#include <cstring>

namespace fj {

const char SIGNATURE[] = "fjgeo";
const size_t SIGNATURE_SIZE = 8;

static void write_signature(std::ofstream &file)
{
  char sign[SIGNATURE_SIZE] = {'\0'};
  strcpy(sign, SIGNATURE);
  file.write(sign, SIGNATURE_SIZE);
}

static bool match_signature(std::ifstream &file, const char *signature)
{
  char sign[SIGNATURE_SIZE] = {'\0'};

  file.read(sign, SIGNATURE_SIZE);
  return strcmp(sign, signature) == 0;
}

template <typename T> inline
static void write_data(std::ofstream &file, const std::string &name, const T &data)
{
  write_(file, name);
  write_(file, data);
}

static void read_data_name(std::ifstream &file, std::string &name)
{
  read_(file, name);
}

GeoOutputFile::GeoOutputFile(const std::string &filename)
{
  file_.open(filename.c_str(), std::fstream::out | std::fstream::binary);
}

GeoOutputFile::~GeoOutputFile()
{
}

int GeoOutputFile::Write(const Geometry &geo)
{
  write_signature(file_);

  write_data(file_, "point::count",    geo.GetPointCount());
  if (geo.HasPointPosition()) {
    write_(file_, "point::position");
    for (Index i = 0; i < geo.GetPointCount(); i++) {
      write_(file_, geo.GetPointPosition(i));
    }
  }
  if (geo.HasPointVelocity()) {
    write_(file_, "point::velocity");
    for (Index i = 0; i < geo.GetPointCount(); i++) {
      write_(file_, geo.GetPointVelocity(i));
    }
  }
  if (geo.HasPointRadius()) {
    write_(file_, "point::radius");
    for (Index i = 0; i < geo.GetPointCount(); i++) {
      write_(file_, geo.GetPointRadius(i));
    }
  }

  return 0;
}

GeoInputFile::GeoInputFile(const std::string &filename)
{
  file_.open(filename.c_str(), std::fstream::in | std::fstream::binary);
}

GeoInputFile::~GeoInputFile()
{
}

int GeoInputFile::Read(Geometry &geo)
{
  if (!match_signature(file_, "fjgeo")) {
    //err_ = -1;
    return -1;
  }

  std::string name;
  for (;;) {
    read_data_name(file_, name);

    if (name == "point::count") {
      Index value = 0;
      read_(file_, value);
      geo.SetPointCount(value);
    }
    else if (name == "point::position") {
      geo.AddPointPosition();
      for (Index i = 0; i < geo.GetPointCount(); i++) {
        Vector value;
        read_(file_, value);
        geo.SetPointPosition(i, value);
      }
    }
    else if (name == "point::velocity") {
      geo.AddPointVelocity();
      for (Index i = 0; i < geo.GetPointCount(); i++) {
        Vector value;
        read_(file_, value);
        geo.SetPointVelocity(i, value);
      }
    }
    else if (name == "point::radius") {
      geo.AddPointRadius();
      for (Index i = 0; i < geo.GetPointCount(); i++) {
        Real value;
        read_(file_, value);
        geo.SetPointRadius(i, value);
      }
    }

    if (!file_) {
      break;
    }
  }
  geo.ComputeBounds();

  return 0;
}

} // namespace xxx
