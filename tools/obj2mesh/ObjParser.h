// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef OBJPARSER_H
#define OBJPARSER_H

#include <istream>

namespace obj {

class ObjParser {
public:
  ObjParser() {}
  virtual ~ObjParser() {}

  int Parse(std::istream &stream);

private:
  virtual void read_v (int ncomponents, double x, double y, double z, double w) {}
  virtual void read_vt(int ncomponents, double x, double y, double z, double w) {}
  virtual void read_vn(int ncomponents, double x, double y, double z, double w) {}

  virtual void read_f(long index_count,
      const long *v_indices,
      const long *vt_indices,
      const long *vn_indices) {}
};

} // namespace xxx

#endif // XXX_H
