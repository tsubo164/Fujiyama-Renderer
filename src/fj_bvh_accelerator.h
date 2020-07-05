// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_BVH_ACCELERATOR_H
#define FJ_BVH_ACCELERATOR_H

#include "fj_accelerator.h"

namespace fj {

class BVHNode;

class BVHAccelerator : public Accelerator {
public:
  BVHAccelerator();
  ~BVHAccelerator();

public:
  virtual int build();
  virtual bool intersect(const Ray &ray, Real time, Intersection *isect) const;
  virtual const char *get_name() const;

  BVHNode *root;
};

} // namespace xxx

#endif // FJ_XXX_H
