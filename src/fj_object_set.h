// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_OBJECT_SET_H
#define FJ_OBJECT_SET_H

#include "fj_primitive_set.h"
#include "fj_types.h"
#include "fj_box.h"

#include <vector>

namespace fj {

class ObjectInstance;
class Intersection;
class Ray;

class ObjectSet : public PrimitiveSet {
public:
  ObjectSet();
  virtual ~ObjectSet();

  Index GetObjectCount() const;

  const ObjectInstance *GetObject(Index index) const;
  void AddObject(const ObjectInstance *obj);

  const Box &GetBounds() const;
  void ComputeBounds();

private:
  virtual bool ray_intersect(Index prim_id, Real time,
      const Ray &ray, Intersection *isect) const;
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const;
  virtual void get_bounds(Box *bounds) const;
  virtual Index get_primitive_count() const;

  std::vector<const ObjectInstance*> objects_;
  Box bounds_;
};

} // namespace xxx

#endif // FJ_XXX_H
