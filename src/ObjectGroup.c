/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "ObjectGroup.h"
#include "VolumeAccelerator.h"
#include "ObjectInstance.h"
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
	struct Accelerator *accelerator;
	struct Array *objects;
	double bounds[6];
};

struct VolumeList {
	struct VolumeAccelerator *accelerator;
	struct Array *volumes;
	double bounds[6];
};

struct ObjectGroup {
	struct ObjectList *surface_list;
	struct VolumeList *volume_list;
};

static struct ObjectList *obj_list_new(void);
static void obj_list_free(struct ObjectList *list);
static void obj_list_add(struct ObjectList *list, const struct ObjectInstance *obj);
static struct VolumeList *vol_list_new(void);
static void vol_list_free(struct VolumeList *list);
static void vol_list_add(struct VolumeList *list, const struct ObjectInstance *obj);

static void object_bounds(const void *prim_set, int prim_id, double *bounds);
static int object_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct Intersection *isect);
static int volume_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct Interval *interval);

struct ObjectGroup *ObjGroupNew(void)
{
	struct ObjectGroup *grp;

	grp = (struct ObjectGroup *) malloc(sizeof(struct ObjectGroup));
	if (grp == NULL)
		return NULL;

	grp->surface_list = obj_list_new();
	if (grp->surface_list == NULL) {
		ObjGroupFree(grp);
		return NULL;
	}

	grp->volume_list = vol_list_new();
	if (grp->volume_list == NULL) {
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
	vol_list_free(grp->volume_list);
	free(grp);
}

void ObjGroupAdd(struct ObjectGroup *grp, const struct ObjectInstance *obj)
{
	if (ObjIsSurface(obj)) {
		obj_list_add(grp->surface_list, obj);
	}
	else if (ObjIsVolume(obj)) {
		vol_list_add(grp->volume_list, obj);
	}
	else {
		printf("fatal error: object is neither surface nor volume\n");
		abort();
	}
}

const struct Accelerator *ObjGroupGetSurfaceAccelerator(const struct ObjectGroup *grp)
{
	return grp->surface_list->accelerator;
}

const struct VolumeAccelerator *ObjGroupGetVolumeAccelerator(const struct ObjectGroup *grp)
{
	return grp->volume_list->accelerator;
}

static void object_bounds(const void *prim_set, int prim_id, double *bounds)
{
	const struct ObjectInstance **objects = (const struct ObjectInstance **) prim_set;
	ObjGetBounds(objects[prim_id], bounds);
}

static int object_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct Intersection *isect)
{
	const struct ObjectInstance **objects = (const struct ObjectInstance **) prim_set;
	return ObjIntersect(objects[prim_id], ray, isect);
}

static int volume_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct Interval *interval)
{
	const struct ObjectInstance **objects = (const struct ObjectInstance **) prim_set;
	const struct ObjectInstance *obj = objects[prim_id];
	double boxhit_tmin;
	double boxhit_tmax;
	double bounds[6];

	ObjGetBounds(obj, bounds);
	if (!BoxRayIntersect(bounds, ray->orig, ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax)) {
		return 0;
	}

	if (!BoxRayIntersect(bounds, ray->orig, ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax)) {
		return 0;
	}

	interval->tmin = boxhit_tmin;
	interval->tmax = boxhit_tmax;
	interval->object = obj;

	return 1;
}

static struct ObjectList *obj_list_new(void)
{
	struct ObjectList *list;

	list = (struct ObjectList *) malloc(sizeof(struct ObjectList));
	if (list == NULL)
		return NULL;

	list->accelerator = AccNew(ACC_BVH);
	if (list->accelerator == NULL) {
		obj_list_free(list);
		return NULL;
	}

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

	if (list->accelerator != NULL)
		AccFree(list->accelerator);

	if (list->objects != NULL)
		ArrFree(list->objects);

	free(list);
}

static void obj_list_add(struct ObjectList *list, const struct ObjectInstance *obj)
{
	double bounds[6];

	if (!ObjIsSurface(obj))
		return;

	ObjGetBounds(obj, bounds);

	ArrPushPointer(list->objects, obj);
	BoxAddBox(list->bounds, bounds);

	AccSetTargetGeometry(list->accelerator,
			list->objects->data,
			list->objects->nelems,
			list->bounds,
			object_ray_intersect,
			object_bounds);
}

static struct VolumeList *vol_list_new(void)
{
	struct VolumeList *list;

	list = (struct VolumeList *) malloc(sizeof(struct VolumeList));
	if (list == NULL)
		return NULL;

	list->accelerator = VolumeAccNew(VOLACC_BVH);
	if (list->accelerator == NULL) {
		vol_list_free(list);
		return NULL;
	}

	list->volumes = ArrNew(sizeof(struct ObjectInstance *));
	if (list->volumes == NULL) {
		vol_list_free(list);
		return NULL;
	}

	BOX3_SET(list->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	return list;
}

static void vol_list_free(struct VolumeList *list)
{
	if (list == NULL)
		return;

	if (list->accelerator != NULL)
		VolumeAccFree(list->accelerator);

	if (list->volumes != NULL)
		ArrFree(list->volumes);

	free(list);
}

static void vol_list_add(struct VolumeList *list, const struct ObjectInstance *obj)
{
	double bounds[6];

	if (!ObjIsVolume(obj))
		return;

	ObjGetBounds(obj, bounds);

	ArrPushPointer(list->volumes, obj);
	BoxAddBox(list->bounds, bounds);

	VolumeAccSetTargetGeometry(list->accelerator,
			list->volumes->data,
			list->volumes->nelems,
			list->bounds,
			volume_ray_intersect,
			object_bounds);
}

