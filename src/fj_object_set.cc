/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_object_set.h"
#include "fj_object_instance.h"

#include <cassert>

namespace fj {

ObjectSet::ObjectSet() : objects_(), bounds_()
{
  BoxReverseInfinite(&bounds_);
}

ObjectSet::~ObjectSet()
{
}

Index ObjectSet::GetObjectCount() const
{
  return objects_.size();
}

const ObjectInstance *ObjectSet::GetObject(Index index) const
{
  assert(index >= 0 && index < static_cast<Index>(objects_.size()));
  return objects_[index];
}

void ObjectSet::AddObject(const ObjectInstance *obj)
{
  Box other_bounds;
  ObjGetBounds(obj, &other_bounds);

  objects_.push_back(obj);

  BoxAddBox(&bounds_, other_bounds);
}

const Box &ObjectSet::GetBounds() const
{
  return bounds_;
}

void ObjectSet::ComputeBounds()
{
  for (int i = 0; i < GetObjectCount(); i++) {
    const ObjectInstance *obj = GetObject(i);
    Box obj_bounds;

    ObjGetBounds(obj, &obj_bounds);
    BoxAddBox(&bounds_, obj_bounds);
  }
}

bool ObjectSet::ray_intersect(Index prim_id, Real time,
    const Ray &ray, Intersection *isect) const
{
  const ObjectInstance *obj = GetObject(prim_id);
  return ObjIntersect(obj, time, &ray, isect);
}

void ObjectSet::get_primitive_bounds(Index prim_id, Box *bounds) const
{
  const ObjectInstance *obj = GetObject(prim_id);
  ObjGetBounds(obj, bounds);
}

void ObjectSet::get_bounds(Box *bounds) const
{
  *bounds = GetBounds();
}

Index ObjectSet::get_primitive_count() const
{
  return GetObjectCount();
}

} // namespace xxx
