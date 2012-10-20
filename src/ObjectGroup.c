/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "ObjectGroup.h"
#include "VolumeAccelerator.h"
#include "ObjectInstance.h"
#include "PrimitiveSet.h"
#include "Accelerator.h"
#include "Interval.h"
#include "Matrix.h"
#include "Array.h"
#include "Ray.h"
#include "Box.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <float.h>

struct ObjectList {
	struct Array *objects;
	double bounds[6];
};

static struct ObjectList *obj_list_new(void);
static void obj_list_free(struct ObjectList *list);
static void obj_list_add(struct ObjectList *list, const struct ObjectInstance *obj);

static void object_bounds(const void *prim_set, int prim_id, double *bounds);
static int object_ray_intersect(const void *prim_set, int prim_id, double time,
		const struct Ray *ray, struct Intersection *isect);
static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
		const struct Ray *ray, struct Interval *interval);

struct ObjectGroup {
	struct ObjectList *surface_list;
	struct ObjectList *volume_list;

	struct Accelerator *surface_acc;
	struct VolumeAccelerator *volume_acc;
};

struct ObjectGroup *ObjGroupNew(void)
{
	struct ObjectGroup *grp;

	grp = (struct ObjectGroup *) malloc(sizeof(struct ObjectGroup));
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

	free(grp);
}

void ObjGroupAdd(struct ObjectGroup *grp, const struct ObjectInstance *obj)
{
	if (ObjIsSurface(obj)) {
		struct PrimitiveSet primset;
		obj_list_add(grp->surface_list, obj);

		MakePrimitiveSet(&primset,
				"ObjectInstance:Surface",
				grp->surface_list->objects->data,
				grp->surface_list->objects->nelems,
				grp->surface_list->bounds,
				object_ray_intersect,
				object_bounds);
		AccSetPrimitiveSet(grp->surface_acc, &primset);
	}
	else if (ObjIsVolume(obj)) {
		obj_list_add(grp->volume_list, obj);

		VolumeAccSetTargetGeometry(grp->volume_acc,
				grp->volume_list->objects->data,
				grp->volume_list->objects->nelems,
				grp->volume_list->bounds,
				volume_ray_intersect,
				object_bounds);
	}
	else {
		printf("fatal error: object is neither surface nor volume\n");
		abort();
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

static void object_bounds(const void *prim_set, int prim_id, double *bounds)
{
	const struct ObjectInstance **objects = (const struct ObjectInstance **) prim_set;
	ObjGetBounds(objects[prim_id], bounds);
}

static int object_ray_intersect(const void *prim_set, int prim_id, double time,
		const struct Ray *ray, struct Intersection *isect)
{
	const struct ObjectInstance **objects = (const struct ObjectInstance **) prim_set;
	return ObjIntersect(objects[prim_id], time, ray, isect);
}

static int volume_ray_intersect(const void *prim_set, int prim_id, double time,
		const struct Ray *ray, struct Interval *interval)
{
	const struct ObjectInstance **objects = (const struct ObjectInstance **) prim_set;
	return ObjVolumeIntersect(objects[prim_id], time, ray, interval);
}

static struct ObjectList *obj_list_new(void)
{
	struct ObjectList *list;

	list = (struct ObjectList *) malloc(sizeof(struct ObjectList));
	if (list == NULL)
		return NULL;

	list->objects = ArrNew(sizeof(struct ObjectInstance *));
	if (list->objects == NULL) {
		obj_list_free(list);
		return NULL;
	}

	BOX3_SET(list->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	return list;
}

static void obj_list_free(struct ObjectList *list)
{
	if (list == NULL)
		return;

	if (list->objects != NULL)
		ArrFree(list->objects);

	free(list);
}

static void obj_list_add(struct ObjectList *list, const struct ObjectInstance *obj)
{
	double bounds[6];

	ObjGetBounds(obj, bounds);

	ArrPushPointer(list->objects, obj);
	BoxAddBox(list->bounds, bounds);
}

