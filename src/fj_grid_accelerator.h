// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_GRIDACCELERATOR_H
#define FJ_GRIDACCELERATOR_H

#include "fj_accelerator.h"
#include "fj_vector.h"
#include "fj_box.h"

#include <vector>

namespace fj {

class Cell;

class GridAccelerator : public Accelerator {
public:
  GridAccelerator();
  ~GridAccelerator();

private:
  virtual int build();
  virtual bool intersect(const Ray &ray, Real time, Intersection *isect) const;
  virtual const char *get_name() const;

  std::vector<Cell*> cells_;
  int ncells_[3];
  Vector cellsize_;
  Box bounds_;
};

} // namespace xxx

#endif // FJ_XXX_H
