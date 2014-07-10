// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "ObjParser.h"

#include <vector>
#include <sstream>
#include <fstream>
#include <string>

namespace obj {

class VectorValue {
public:
  VectorValue() :
      min_ncomponents(3),
      max_ncomponents(3),
      scanned_ncomponents(0),
      x(0.), y(0.), z(0.), w(0.) {}
  ~VectorValue() {}

public:
  int min_ncomponents;
  int max_ncomponents;
  int scanned_ncomponents;
  double x, y, z, w;
};

class ObjIndex {
public:
  ObjIndex() :
      v(0), vt(0), vn(0),
      has_v(false), has_vt(false), has_vn(false) {}
  ~ObjIndex() {}

public:
  long v, vt, vn;
  bool has_v, has_vt, has_vn;
};

class IndexList {
  static const size_t INITIAL_INDEX_ALLOC = 8;
public:
  IndexList() :
      vertex_ (INITIAL_INDEX_ALLOC),
      texture_(INITIAL_INDEX_ALLOC),
      normal_ (INITIAL_INDEX_ALLOC) {}
  ~IndexList() {}

  void Push(const ObjIndex &index)
  {
      if (index.has_v)  vertex_.push_back(index.v);
      if (index.has_vt) texture_.push_back(index.vt);
      if (index.has_vn) normal_.push_back(index.vn);
  }

  void Clear()
  {
    vertex_.clear();
    texture_.clear();
    normal_.clear();
  }

  long Count() const
  {
    return vertex_.size();
  }

  const long *GetVertex()  const { return vertex_.empty() ? NULL : &vertex_[0]; }
  const long *GetTexture() const { return texture_.empty() ? NULL : &texture_[0]; }
  const long *GetNormal()  const { return normal_.empty() ? NULL : &normal_[0]; }

private:
  std::vector<long> vertex_;
  std::vector<long> texture_;
  std::vector<long> normal_;
};

inline std::istream &operator>>(std::istream &is, ObjIndex &index)
{
  is >> index.v;
  index.has_v = true;
  if (is.get() != '/') {
    return is;
  }

  if (is.peek() == '/') {
    is.get(); // skip '/'
    is >> index.vn;
    index.has_vn = true;
    return is;
  }

  is >> index.vt;
  index.has_vt = true;
  if (is.get() != '/') {
    return is;
  }

  is >> index.vn;
  index.has_vn = true;
  return is;
}

inline std::istream &operator>>(std::istream &is, VectorValue &vector)
{
  double v[4] = {0, 0, 0, 0};
  int i;

  for (i = 0; i < 4; i++) {
    is >> v[i];
    if (!is) {
      is.clear();
      break;
    }
  }

  vector.scanned_ncomponents = i;
  vector.x = v[0];
  vector.y = v[1];
  vector.z = v[2];
  vector.w = v[3];

  return is;
}

static std::string trimed(const std::string &line)
{
  const char white_spaces[] = " \t\f\v\n\r";
  size_t f = line.find_first_not_of(white_spaces);
  size_t l = line.find_last_not_of(white_spaces);

  if (f == std::string::npos) f = 0;

  return line.substr(f, l - f + 1);
}

int ObjParser::Parse(std::istream &stream)
{
  IndexList list;
  std::string line;

  while (getline(stream, line)) {
    std::istringstream iss(trimed(line));
    std::string tag;
    iss >> tag;

    if (tag == "v") {
      VectorValue vector;
      vector.min_ncomponents = 3;
      vector.max_ncomponents = 4;
      iss >> vector;

      read_v(vector.scanned_ncomponents, vector.x, vector.y, vector.z, vector.w);
    }
    else if (tag == "vt") {
      VectorValue vector;
      vector.min_ncomponents = 1;
      vector.max_ncomponents = 3;
      iss >> vector;

      read_vt(vector.scanned_ncomponents, vector.x, vector.y, vector.z, vector.w);
    }
    else if (tag == "vn") {
      VectorValue vector;
      vector.min_ncomponents = 3;
      vector.max_ncomponents = 3;
      iss >> vector;

      read_vn(vector.scanned_ncomponents, vector.x, vector.y, vector.z, vector.w);
    }
    else if (tag == "f") {
      list.Clear();
      while (iss) {
        ObjIndex index;
        iss >> index;
        // TODO make relative_to_abs
        index.v--;
        index.vt--;
        index.vn--;
        list.Push(index);
      }

      const long *vertex  = list.GetVertex();
      const long *texture = list.GetTexture();
      const long *normal  = list.GetVertex();

      read_f(list.Count(), vertex, texture, normal);
    }
  }

  return 0;
}

} // namespace xxx
