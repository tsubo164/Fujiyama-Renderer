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
#include "fj_matrix.h"
#include "fj_memory.h"
#include "fj_ray.h"
#include "fj_box.h"

#include <vector>
#include <cassert>
#include <cstdio>
#include <cfloat>

namespace fj {

class ObjectList {
public:
  ObjectList() : objects(), bounds()
  {
    BoxReverseInfinite(&bounds);
  }
  ~ObjectList()
  {
  }

  Index GetObjectCount() const
  {
    return objects.size();
  }
  const Box &GetBounds() const
  {
    return bounds;
  }
  const ObjectInstance *GetObject(Index index) const
  {
    // TODO CHECK RANGE
    return objects[index];
  }
  void AddObject(const ObjectInstance *obj)
  {
    Box other_bounds;
    ObjGetBounds(obj, &other_bounds);

    objects.push_back(obj);

    BoxAddBox(&bounds, other_bounds);
  }
  void ComputeBounds()
  {
    for (int i = 0; i < GetObjectCount(); i++) {
      const ObjectInstance *obj = GetObject(i);
      struct Box obj_bounds;

      ObjGetBounds(obj, &obj_bounds);
      BoxAddBox(&bounds, obj_bounds);
    }
  }

private:
  std::vector<const ObjectInstance*> objects;
  Box bounds;
};

static void object_bounds(const void *prim_set, int prim_id, struct Box *bounds);
static int object_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Intersection *isect);
static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Interval *interval);
static void object_list_bounds(const void *prim_set, struct Box *bounds);
static int object_count(const void *prim_set);

class ObjectGroup {
public:
  ObjectGroup();
  ~ObjectGroup();

  ObjectList surface_list;
  ObjectList volume_list;

  struct Accelerator *surface_acc;
  struct VolumeAccelerator *volume_acc;
};

ObjectGroup::ObjectGroup() : 
    surface_list(),
    volume_list(),
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
  if (ObjIsSurface(obj)) {
    struct PrimitiveSet primset;
    grp->surface_list.AddObject(obj);

    MakePrimitiveSet(&primset,
        "ObjectInstance:Surface",
        &grp->surface_list,
        object_ray_intersect,
        object_bounds,
        object_list_bounds,
        object_count);
    AccSetPrimitiveSet(grp->surface_acc, &primset);
  }
  else if (ObjIsVolume(obj)) {
    grp->volume_list.AddObject(obj);

    VolumeAccSetTargetGeometry(grp->volume_acc,
        &grp->volume_list,
        grp->volume_list.GetObjectCount(),
        &grp->volume_list.GetBounds(),
        volume_ray_intersect,
        object_bounds);
  }
}

const struct Accelerator *ObjGroupGetSurfaceAccelerator(const struct ObjectGroup *grp)
{
  return grp->surface_acc;
}

const struct VolumeAccelerator *ObjGroupGetVolumeAccelerator(const struct ObjectGroup *grp)
{
  return grp->volume_acc;
}

void ObjGroupComputeBounds(struct ObjectGroup *grp)
{
  grp->surface_list.ComputeBounds();
  grp->volume_list.ComputeBounds();
}

static void object_bounds(const void *prim_set, int prim_id, struct Box *bounds)
{
  const struct ObjectList *list = (const struct ObjectList *) prim_set;
  const struct ObjectInstance *obj = list->GetObject(prim_id);
  ObjGetBounds(obj, bounds);
}

static int object_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Intersection *isect)
{
  const struct ObjectList *list = (const struct ObjectList *) prim_set;
  const struct ObjectInstance *obj = list->GetObject(prim_id);
  return ObjIntersect(obj, time, ray, isect);
}

static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Interval *interval)
{
  const struct ObjectList *list = (const struct ObjectList *) prim_set;
  const struct ObjectInstance *obj = list->GetObject(prim_id);
  return ObjVolumeIntersect(obj, time, ray, interval);
}

static void object_list_bounds(const void *prim_set, struct Box *bounds)
{
  const struct ObjectList *list = (const struct ObjectList *) prim_set;
  *bounds = list->GetBounds();
}

static int object_count(const void *prim_set)
{
  const struct ObjectList *list = (const struct ObjectList *) prim_set;
  return list->GetObjectCount();
}

} // namespace xxx
