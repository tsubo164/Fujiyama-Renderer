/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_object_group.h"
#include "fj_volume_accelerator.h"
#include "fj_object_instance.h"
#include "fj_primitive_set.h"
#include "fj_accelerator.h"
#include "fj_interval.h"
#include "fj_ray.h"
#include "fj_box.h"

#include <vector>

namespace fj {

class ObjectSet : public PrimitiveSet {
public:
  ObjectSet();
  ~ObjectSet();

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
  // TODO CHECK RANGE
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
    struct Box obj_bounds;

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

static void volume_bounds(const void *prim_set, int prim_id, struct Box *bounds);
static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Interval *interval);

class ObjectGroup {
public:
  ObjectGroup();
  ~ObjectGroup();

  void AddObject(const ObjectInstance *obj);
  const Accelerator *GetSurfaceAccelerator() const;
  const VolumeAccelerator *GetVolumeAccelerator() const;

  void ComputeBounds();

public:
  ObjectSet surface_set;
  ObjectSet volume_set;

  struct Accelerator *surface_acc;
  struct VolumeAccelerator *volume_acc;
};

ObjectGroup::ObjectGroup() : 
    surface_set(),
    volume_set(),
    surface_acc(NULL),
    volume_acc(NULL)
{
  surface_acc = AccNew(ACC_BVH);
  volume_acc = VolumeAccNew(VOLACC_BVH);
}

ObjectGroup::~ObjectGroup()
{
  AccFree(surface_acc);
  VolumeAccFree(volume_acc);
}

void ObjectGroup::AddObject(const ObjectInstance *obj)
{
  if (ObjIsSurface(obj)) {
    surface_set.AddObject(obj);
    AccSetPrimitiveSetPointer(surface_acc, &surface_set);
  }
  else if (ObjIsVolume(obj)) {
    volume_set.AddObject(obj);

    VolumeAccSetTargetGeometry(volume_acc,
        &volume_set,
        volume_set.GetObjectCount(),
        &volume_set.GetBounds(),
        volume_ray_intersect,
        volume_bounds);
  }
}

const Accelerator *ObjectGroup::GetSurfaceAccelerator() const
{
  return surface_acc;
}

const VolumeAccelerator *ObjectGroup::GetVolumeAccelerator() const
{
  return volume_acc;
}

void ObjectGroup::ComputeBounds()
{
  surface_set.ComputeBounds();
  volume_set.ComputeBounds();
}

struct ObjectGroup *ObjGroupNew(void)
{
  return new ObjectGroup();
}

void ObjGroupFree(struct ObjectGroup *grp)
{
  delete grp;
}

void ObjGroupAdd(struct ObjectGroup *grp, const struct ObjectInstance *obj)
{
  grp->AddObject(obj);
}

const struct Accelerator *ObjGroupGetSurfaceAccelerator(const struct ObjectGroup *grp)
{
  return grp->GetSurfaceAccelerator();
}

const struct VolumeAccelerator *ObjGroupGetVolumeAccelerator(const struct ObjectGroup *grp)
{
  return grp->GetVolumeAccelerator();
}

void ObjGroupComputeBounds(struct ObjectGroup *grp)
{
  grp->ComputeBounds();
}

static void volume_bounds(const void *prim_set, int prim_id, struct Box *bounds)
{
  const ObjectSet *objset = (const ObjectSet *) prim_set;
  const struct ObjectInstance *obj = objset->GetObject(prim_id);
  ObjGetBounds(obj, bounds);
}

static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Interval *interval)
{
  const ObjectSet *objset = (const ObjectSet *) prim_set;
  const struct ObjectInstance *obj = objset->GetObject(prim_id);
  return ObjVolumeIntersect(obj, time, ray, interval);
}

} // namespace xxx
