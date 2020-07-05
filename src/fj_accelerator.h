// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_ACCERALATOR_H
#define FJ_ACCERALATOR_H

#include "fj_types.h"
#include "fj_box.h"

namespace fj {

class Intersection;
class PrimitiveSet;
class Ray;

class Accelerator {
public:
  Accelerator();
  virtual ~Accelerator();

  Real GetBoundsPadding() const;
  const Box &GetBounds() const;
  const char *GetName() const;
  bool HasBuilt() const;

  void ComputeBounds();
  void SetPrimitiveSet(PrimitiveSet *primset);
  int Build();
  bool Intersect(const Ray &ray, Real time, Intersection *isect) const;

private:
  virtual int build() = 0;
  virtual bool intersect(const Ray &ray, Real time, Intersection *isect) const = 0;
  virtual const char *get_name() const = 0;

  Box bounds_;
  bool has_built_;

  PrimitiveSet *primset_;

protected:
  // TODO PrimitiveSet might have to own Accelerator
  const PrimitiveSet *GetPrimitiveSet() const { return primset_; }
};

} // namespace xxx

#endif // FJ_XXX_H
