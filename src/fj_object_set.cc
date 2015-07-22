// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

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
  other_bounds = obj->GetBounds();

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

    obj_bounds = obj->GetBounds();
    BoxAddBox(&bounds_, obj_bounds);
  }
}

bool ObjectSet::ray_intersect(Index prim_id, const Ray &ray,
    Real time, Intersection *isect) const
{
  const ObjectInstance *obj = GetObject(prim_id);
  return obj->RayIntersect(ray, time, isect);
}

void ObjectSet::get_primitive_bounds(Index prim_id, Box *bounds) const
{
  const ObjectInstance *obj = GetObject(prim_id);
  *bounds = obj->GetBounds();
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
