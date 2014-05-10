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
#include "fj_array.h"
#include "fj_ray.h"
#include "fj_box.h"

#include <assert.h>
#include <stdio.h>
#include <float.h>

struct ObjectList {
  struct Array *objects;
  struct Box bounds;
};

static struct ObjectList *obj_list_new(void);
static void obj_list_free(struct ObjectList *list);
static void obj_list_add(struct ObjectList *list, const struct ObjectInstance *obj);
static const struct ObjectInstance *get_object(const struct ObjectList *list, int index);

static void object_bounds(const void *prim_set, int prim_id, struct Box *bounds);
static int object_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Intersection *isect);
static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Interval *interval);
static void object_list_bounds(const void *prim_set, struct Box *bounds);
static int object_count(const void *prim_set);

struct ObjectGroup {
  struct ObjectList *surface_list;
  struct ObjectList *volume_list;

  struct Accelerator *surface_acc;
  struct VolumeAccelerator *volume_acc;
};

struct ObjectGroup *ObjGroupNew(void)
{
  struct ObjectGroup *grp = FJ_MEM_ALLOC(struct ObjectGroup);
  if (grp == NULL)
    return NULL;

  grp->surface_list = NULL;
  grp->volume_list = NULL;
  grp->surface_acc = NULL;
  grp->volume_acc = NULL;

  grp->surface_list = obj_list_new();
  if (grp->surface_list == NULL) {
    ObjGroupFree(grp);
    return NULL;
  }

  grp->volume_list = obj_list_new();
  if (grp->volume_list == NULL) {
    ObjGroupFree(grp);
    return NULL;
  }

  grp->surface_acc = AccNew(ACC_BVH);
  if (grp->surface_acc == NULL) {
    ObjGroupFree(grp);
    return NULL;
  }

  grp->volume_acc = VolumeAccNew(VOLACC_BVH);
  if (grp->volume_acc == NULL) {
    ObjGroupFree(grp);
    return NULL;
  }

  return grp;
}

void ObjGroupFree(struct ObjectGroup *grp)
{
  if (grp == NULL)
    return;

  obj_list_free(grp->surface_list);
  obj_list_free(grp->volume_list);

  AccFree(grp->surface_acc);
  VolumeAccFree(grp->volume_acc);

  FJ_MEM_FREE(grp);
}

void ObjGroupAdd(struct ObjectGroup *grp, const struct ObjectInstance *obj)
{
  if (ObjIsSurface(obj)) {
    struct PrimitiveSet primset;
    obj_list_add(grp->surface_list, obj);

    MakePrimitiveSet(&primset,
        "ObjectInstance:Surface",
        grp->surface_list,
        object_ray_intersect,
        object_bounds,
        object_list_bounds,
        object_count);
    AccSetPrimitiveSet(grp->surface_acc, &primset);
  }
  else if (ObjIsVolume(obj)) {
    obj_list_add(grp->volume_list, obj);

    VolumeAccSetTargetGeometry(grp->volume_acc,
        grp->volume_list,
        grp->volume_list->objects->nelems,
        &grp->volume_list->bounds,
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
  int N = 0;
  int i;

  N = grp->surface_list->objects->nelems;
  for (i = 0; i < N; i++) {
    const struct ObjectInstance *obj = get_object(grp->surface_list, i);
    struct Box bounds = {{0}};

    ObjGetBounds(obj, &bounds);
    BoxAddBox(&grp->surface_list->bounds, &bounds);
  }

  N = grp->volume_list->objects->nelems;
  for (i = 0; i < N; i++) {
    const struct ObjectInstance *obj = get_object(grp->volume_list, i);
    struct Box bounds = {{0}};

    ObjGetBounds(obj, &bounds);
    BoxAddBox(&grp->volume_list->bounds, &bounds);
  }
}

static void object_bounds(const void *prim_set, int prim_id, struct Box *bounds)
{
  const struct ObjectList *list = (const struct ObjectList *) prim_set;
  const struct ObjectInstance *obj = get_object(list, prim_id);
  ObjGetBounds(obj, bounds);
}

static int object_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Intersection *isect)
{
  const struct ObjectList *list = (const struct ObjectList *) prim_set;
  const struct ObjectInstance *obj = get_object(list, prim_id);
  return ObjIntersect(obj, time, ray, isect);
}

static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Interval *interval)
{
  const struct ObjectList *list = (const struct ObjectList *) prim_set;
  const struct ObjectInstance *obj = get_object(list, prim_id);
  return ObjVolumeIntersect(obj, time, ray, interval);
}

static void object_list_bounds(const void *prim_set, struct Box *bounds)
{
  const struct ObjectList *list = (const struct ObjectList *) prim_set;
  *bounds = list->bounds;
}

static int object_count(const void *prim_set)
{
  const struct ObjectList *list = (const struct ObjectList *) prim_set;
  return list->objects->nelems;
}

static struct ObjectList *obj_list_new(void)
{
  struct ObjectList *list;

  list = FJ_MEM_ALLOC(struct ObjectList);
  if (list == NULL)
    return NULL;

  list->objects = ArrNew(sizeof(struct ObjectInstance *));
  if (list->objects == NULL) {
    obj_list_free(list);
    return NULL;
  }

  BOX3_SET(&list->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

  return list;
}

static void obj_list_free(struct ObjectList *list)
{
  if (list == NULL)
    return;

  if (list->objects != NULL)
    ArrFree(list->objects);

  FJ_MEM_FREE(list);
}

static void obj_list_add(struct ObjectList *list, const struct ObjectInstance *obj)
{
  struct Box bounds = {{0}};

  ObjGetBounds(obj, &bounds);

  ArrPushPointer(list->objects, obj);
  BoxAddBox(&list->bounds, &bounds);
}

static const struct ObjectInstance *get_object(const struct ObjectList *list, int index)
{
  const struct ObjectInstance **objects =
      (const struct ObjectInstance **) list->objects->data;
  return objects[index];
}

