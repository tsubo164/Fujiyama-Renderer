// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef OBJPARSER_H
#define OBJPARSER_H

#include <istream>
#include <string>
#include <vector>

namespace obj {

class ObjParser {
public:
  ObjParser();
  virtual ~ObjParser();

  int Parse(std::istream &stream);

private:
  virtual void read_v (int ncomponents, double x, double y, double z, double w) {}
  virtual void read_vt(int ncomponents, double x, double y, double z, double w) {}
  virtual void read_vn(int ncomponents, double x, double y, double z, double w) {}

  virtual void read_f(long index_count,
      const long *v_indices,
      const long *vt_indices,
      const long *vn_indices) {}

// TODO is this better?
#if 0
  virtual void read_f(long index_count,
      const std::vector<long> &v_indices,
      const std::vector<long> &vt_indices,
      const std::vector<long> &vn_indices) {}
#endif

  virtual void read_g(const std::vector<std::string> &group_name_list) {}

  long v_count_;
  long vt_count_;
  long vn_count_;
  long f_count_;
};

} // namespace xxx

#endif // XXX_H
