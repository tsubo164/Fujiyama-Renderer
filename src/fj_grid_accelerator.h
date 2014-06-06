/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_GRIDACCELERATOR_H
#define FJ_GRIDACCELERATOR_H

#include "fj_accelerator.h"
#include "fj_box.h"

namespace fj {

extern void GetGridAcceleratorFunction(struct Accelerator *acc);

struct Cell;

class GridAccelerator : public Accelerator {
public:
  GridAccelerator();
  ~GridAccelerator();

public:
  virtual int build();
  virtual bool intersect(const Ray &ray, Real time, Intersection *isect) const;

  struct Cell **cells;
  int ncells[3];
  double cellsize[3];
  struct Box bounds;
};

} // namespace xxx

#endif /* FJ_XXX_H */
