/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_object_group.h"
#include "fj_volume_accelerator.h"
#include "fj_bvh_accelerator.h"
#include "fj_object_instance.h"
#include "fj_accelerator.h"
#include "fj_interval.h"
#include "fj_ray.h"

#include <cassert>

namespace fj {

static void volume_bounds(const void *prim_set, int prim_id, Box *bounds);
static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
    const Ray *ray, Interval *interval);

ObjectGroup::ObjectGroup() :
    surface_set(),
    volume_set(),
    surface_acc(NULL),
    volume_acc(NULL)
{
  surface_acc = new BVHAccelerator();
  volume_acc = VolumeAccNew(VOLACC_BVH);
}

ObjectGroup::~ObjectGroup()
{
  delete surface_acc;
  VolumeAccFree(volume_acc);
}

void ObjectGroup::AddObject(const ObjectInstance *obj)
{
  if (ObjIsSurface(obj)) {
    surface_set.AddObject(obj);
    surface_acc->SetPrimitiveSet(&surface_set);
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

ObjectGroup *ObjGroupNew(void)
{
  return new ObjectGroup();
}

void ObjGroupFree(ObjectGroup *grp)
{
  delete grp;
}

static void volume_bounds(const void *prim_set, int prim_id, Box *bounds)
{
  const ObjectSet *objset = (const ObjectSet *) prim_set;
  const ObjectInstance *obj = objset->GetObject(prim_id);
  ObjGetBounds(obj, bounds);
}

static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
    const Ray *ray, Interval *interval)
{
  const ObjectSet *objset = (const ObjectSet *) prim_set;
  const ObjectInstance *obj = objset->GetObject(prim_id);
  return ObjVolumeIntersect(obj, time, ray, interval);
}

} // namespace xxx
