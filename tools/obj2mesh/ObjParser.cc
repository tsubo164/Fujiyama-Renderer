// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "ObjParser.h"

#include <sstream>
#include <fstream>
#include <iostream>

namespace obj {

class Vertex {
public:
  Vertex() :
      data_count(0),
      x(0.), y(0.), z(0.), w(0.) {}
  Vertex(int count, double xx, double yy, double zz, double ww) :
      data_count(count),
      x(xx), y(yy), z(zz), w(ww) {}
  ~Vertex() {}

public:
  int data_count;
  double x, y, z, w;
};

class Triplet {
public:
  Triplet() :
      v(0), vt(0), vn(0),
      has_v(false), has_vt(false), has_vn(false) {}
  ~Triplet() {}

public:
  long v, vt, vn;
  bool has_v, has_vt, has_vn;
};

class TripletList {
  static const size_t INITIAL_INDEX_ALLOC = 8;
public:
  TripletList() :
      vertex_ (INITIAL_INDEX_ALLOC),
      texture_(INITIAL_INDEX_ALLOC),
      normal_ (INITIAL_INDEX_ALLOC) {}
  ~TripletList() {}

  void Push(const Triplet &triplet)
  {
      if (triplet.has_v)  vertex_.push_back(triplet.v);
      if (triplet.has_vt) texture_.push_back(triplet.vt);
      if (triplet.has_vn) normal_.push_back(triplet.vn);
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

static bool get_triplet(std::istream &is, Triplet &triplet)
{
  Triplet tri;

  is >> tri.v;
  if (!is) {
    return false;
  }

  tri.has_v = true;
  if (is.get() != '/') {
    triplet = tri;
    return true;
  }

  if (is.peek() == '/') {
    is.get(); // skip '/'
    is >> tri.vn;
    tri.has_vn = true;
    triplet = tri;
    return true;
  }

  is >> tri.vt;
  tri.has_vt = true;
  if (is.get() != '/') {
    triplet = tri;
    return true;
  }

  is >> tri.vn;
  tri.has_vn = true;

  triplet = tri;
  return true;
}

static Vertex get_vector(std::istream &is)
{
  double v[4] = {0, 0, 0, 0};
  int data_count = 0;

  for (int i = 0; i < 4; i++) {
    is >> v[i];
    if (!is) {
      is.clear();
      break;
    }
    data_count++;
  }

  return Vertex(
      data_count,
      v[0],
      v[1],
      v[2],
      v[3]);
}

static std::string trimed(const std::string &line)
{
  const char white_spaces[] = " \t\f\v\n\r";
  size_t f = line.find_first_not_of(white_spaces);
  size_t l = line.find_last_not_of(white_spaces);

  if (f == std::string::npos) f = 0;

  return line.substr(f, l - f + 1);
}

static long reindicing_single(long total, long index)
{
  if (index > 0) {
    return index - 1;
  } else if (index < 0) {
    return index + total;
  } else {
    return 0;
  }
}

static void reindicing(long v_count, long vt_count, long vn_count, Triplet *triplet)
{
  triplet->v  = reindicing_single(v_count,  triplet->v);
  triplet->vt = reindicing_single(vt_count, triplet->vt);
  triplet->vn = reindicing_single(vn_count, triplet->vn);
}

ObjParser::ObjParser() :
    v_count_(0),
    vt_count_(0),
    vn_count_(0),
    f_count_(0)
{
}

ObjParser::~ObjParser()
{
}

int ObjParser::Parse(std::istream &stream)
{
  TripletList triplets;
  std::string line;

  while (getline(stream, line)) {
    std::istringstream iss(trimed(line));
    std::string tag;
    iss >> tag;

    if (tag == "v") {
      const Vertex vertex = get_vector(iss);
      read_v(vertex.data_count, vertex.x, vertex.y, vertex.z, vertex.w);
      v_count_++;
    }
    else if (tag == "vt") {
      const Vertex vertex = get_vector(iss);
      read_vt(vertex.data_count, vertex.x, vertex.y, vertex.z, vertex.w);
      vt_count_++;
    }
    else if (tag == "vn") {
      const Vertex vertex = get_vector(iss);
      read_vn(vertex.data_count, vertex.x, vertex.y, vertex.z, vertex.w);
      vn_count_++;
    }
    else if (tag == "f") {
      triplets.Clear();

      for (;;) {
        Triplet triplet;
        if(!get_triplet(iss, triplet)) {
          break;
        }

        reindicing(v_count_, vt_count_, vn_count_, &triplet);
        triplets.Push(triplet);
      }

      read_f(
          triplets.Count(),
          triplets.GetVertex(),
          triplets.GetTexture(),
          triplets.GetNormal());
      f_count_++;
    }
    else if (tag == "g") {
      std::vector<std::string> groups;
      while (iss) {
        std::string name;
        iss >> name;
        if (!iss.fail()) {
          groups.push_back(name);
        }
      }

      read_g(groups);
    }
  }

  return 0;
}

} // namespace xxx
