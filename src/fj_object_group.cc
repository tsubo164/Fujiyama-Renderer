// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_object_group.h"
#include "fj_volume_accelerator.h"
#include "fj_bvh_accelerator.h"
#include "fj_object_instance.h"

#include <cassert>

namespace fj {

class Interval;
class Ray;

//TODO remove these
static void volume_bounds(const void *prim_set, int prim_id, Box *bounds);
static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
    const Ray *ray, Interval *interval);

ObjectGroup::ObjectGroup() :
    surface_set_(),
    volume_set_(),
    surface_set_acc_(NULL),
    volume_set_acc_(NULL)
{
  surface_set_acc_ = new BVHAccelerator();
  volume_set_acc_ = VolumeAccNew(VOLACC_BVH);
}

ObjectGroup::~ObjectGroup()
{
  delete surface_set_acc_;
  VolumeAccFree(volume_set_acc_);
}

void ObjectGroup::AddObject(const ObjectInstance *obj)
{
  if (obj->IsSurface()) {
    surface_set_.AddObject(obj);
    surface_set_acc_->SetPrimitiveSet(&surface_set_);
  }
  else if (obj->IsVolume()) {
    volume_set_.AddObject(obj);

    VolumeAccSetTargetGeometry(volume_set_acc_,
        &volume_set_,
        volume_set_.GetObjectCount(),
        &volume_set_.GetBounds(),
        volume_ray_intersect,
        volume_bounds);
  }
}

const Accelerator *ObjectGroup::GetSurfaceAccelerator() const
{
  return surface_set_acc_;
}

const VolumeAccelerator *ObjectGroup::GetVolumeAccelerator() const
{
  return volume_set_acc_;
}

void ObjectGroup::ComputeBounds()
{
  surface_set_.ComputeBounds();
  volume_set_.ComputeBounds();

  surface_set_acc_->ComputeBounds();

  //TODO volume_set_acc_->ComputeBounds();
  VolumeAccSetTargetGeometry(volume_set_acc_,
      &volume_set_,
      volume_set_.GetObjectCount(),
      &volume_set_.GetBounds(),
      volume_ray_intersect,
      volume_bounds);
}

ObjectGroup *ObjGroupNew()
{
  return new ObjectGroup();
}

void ObjGroupFree(ObjectGroup *group)
{
  delete group;
}

static void volume_bounds(const void *prim_set, int prim_id, Box *bounds)
{
  const ObjectSet *objset = (const ObjectSet *) prim_set;
  const ObjectInstance *obj = objset->GetObject(prim_id);
  *bounds = obj->GetBounds();
}

static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
    const Ray *ray, Interval *interval)
{
  const ObjectSet *objset = (const ObjectSet *) prim_set;
  const ObjectInstance *obj = objset->GetObject(prim_id);
  return obj->RayVolumeIntersect(*ray, time, interval);
}

} // namespace xxx
