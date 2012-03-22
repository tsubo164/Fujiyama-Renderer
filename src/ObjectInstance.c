/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "ObjectInstance.h"
#include "LocalGeometry.h"
#include "Accelerator.h"
#include "Vector.h"
/* XXX TEST */
#include "Volume.h"
#include "Matrix.h"
#include "Array.h"
#include "Box.h"
#include "Ray.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <float.h>

struct ObjectInstance {
	/* geometric properties */
	const struct Accelerator *acc;
	double bounds[6];

	/* transform properties */
	struct Matrix object_to_world;
	struct Matrix world_to_object;
	double translate[3];
	double rotate[3];
	double scale[3];
	int xform_order;
	int rotate_order;

	/* non-geometric properties */
	const struct Shader *shader;
	const struct Light **target_lights;
	int n_target_lights;
	const struct ObjectGroup *reflection_target;
	const struct ObjectGroup *refraction_target;
};

/* internal uses only */
struct ObjectList {
	struct Accelerator *accelerator;
	struct Array *objects;
	double bounds[6];
};

struct ObjectGroup {
#if 0
	struct Accelerator *accelerator;
	struct Array *objects;
	double bounds[6];
#endif
	struct ObjectList *surface_list;
	struct ObjectList *volume_list;
};

static void update_matrix_and_bounds(struct ObjectInstance *obj);
#if 0
static void update_group_accelerator(struct ObjectGroup *grp);
#endif
static void update_object_bounds(struct ObjectInstance *obj);

static void object_bounds(const void *prim_set, int prim_id, double *bounds);
static int object_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);

/* ObjectList interfaces */
static struct ObjectList *obj_list_new(void);
static void obj_list_free(struct ObjectList *list);
static void obj_list_add(struct ObjectList *list, const struct ObjectInstance *obj);

/* ObjectInstance interfaces */
struct ObjectInstance *ObjNew(const struct Accelerator *acc)
{
	struct ObjectInstance *obj;

	obj = (struct ObjectInstance *) malloc(sizeof(struct ObjectInstance));
	if (obj == NULL)
		return NULL;

	obj->acc = acc;

	MatIdentity(&obj->object_to_world);
	MatIdentity(&obj->world_to_object);

	VEC3_SET(obj->translate, 0, 0, 0);
	VEC3_SET(obj->rotate, 0, 0, 0);
	VEC3_SET(obj->scale, 1, 1, 1);
	obj->xform_order = ORDER_SRT;
	obj->rotate_order = ORDER_XYZ;

	obj->shader = NULL;
	obj->target_lights = NULL;
	obj->n_target_lights = 0;

	obj->reflection_target = NULL;
	obj->refraction_target = NULL;
	update_matrix_and_bounds(obj);

	return obj;
}

void ObjFree(struct ObjectInstance *obj)
{
	if (obj == NULL)
		return;
	free(obj);
}

void ObjSetTranslate(struct ObjectInstance *obj, double tx, double ty, double tz)
{
	VEC3_SET(obj->translate, tx, ty, tz);
	update_matrix_and_bounds(obj);
}

void ObjSetRotate(struct ObjectInstance *obj, double rx, double ry, double rz)
{
	VEC3_SET(obj->rotate, rx, ry, rz);
	update_matrix_and_bounds(obj);
}

void ObjSetScale(struct ObjectInstance *obj, double sx, double sy, double sz)
{
	VEC3_SET(obj->scale, sx, sy, sz);
	update_matrix_and_bounds(obj);
}

void ObjSetTransformOrder(struct ObjectInstance *obj, int order)
{
	assert(IsValidTransformOrder(order));
	obj->xform_order = order;
	update_matrix_and_bounds(obj);
}

void ObjSetRotateOrder(struct ObjectInstance *obj, int order)
{
	assert(IsValidRotatedOrder(order));
	obj->rotate_order = order;
	update_matrix_and_bounds(obj);
}

void ObjSetShader(struct ObjectInstance *obj, const struct Shader *shader)
{
	obj->shader = shader;
}

void ObjSetLightList(struct ObjectInstance *obj, const struct Light **lights, int count)
{
	obj->target_lights = lights;
	obj->n_target_lights = count;
}

void ObjSetReflectTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp)
{
	assert(grp != NULL);
	obj->reflection_target = grp;
}

void ObjSetRefractTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp)
{
	assert(grp != NULL);
	obj->refraction_target = grp;
}

const struct ObjectGroup *ObjGetReflectTarget(const struct ObjectInstance *obj)
{
	return obj->reflection_target;
}

const struct ObjectGroup *ObjGetRefractTarget(const struct ObjectInstance *obj)
{
	return obj->refraction_target;
}

const struct Shader *ObjGetShader(const struct ObjectInstance *obj)
{
	return obj->shader;
}

const struct Light **ObjGetLightList(const struct ObjectInstance *obj)
{
	return obj->target_lights;
}

int ObjGetLightCount(const struct ObjectInstance *obj)
{
	return obj->n_target_lights;
}

int ObjIntersect(const struct ObjectInstance *obj, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	int hit;
	struct Ray ray_in_objspace;

	if (obj->acc == NULL)
		return 0;

	/* transform ray to object space */
	ray_in_objspace = *ray;
	TransformPoint(ray_in_objspace.orig, &obj->world_to_object);
	TransformVector(ray_in_objspace.dir, &obj->world_to_object);

	hit = AccIntersect(obj->acc, &ray_in_objspace, isect, t_hit);
	if (!hit)
		return 0;
#if 0
	{
		int primtype = AccGetPrimitiveType(obj->acc);
		hit = 0;
		if (primtype == ACC_PRIM_SURFACE) {
			hit = AccIntersect(obj->acc, &ray_in_objspace, isect, t_hit);
		}
		else if (primtype == ACC_PRIM_VOLUME) {
			struct Volume *volume = AccGetVolume(obj->acc, 0);
			hit = VolSample(volume, &ray_in_objspace, isect, t_hit);
		}
		if (!hit)
			return 0;
	}
#endif

	/* transform intersection back to world space */
	TransformPoint(isect->P, &obj->object_to_world);
	TransformVector(isect->N, &obj->object_to_world);
	VEC3_NORMALIZE(isect->N);

	/* TODO should make TransformLocalGeometry? */
	TransformVector(isect->dPds, &obj->object_to_world);
	TransformVector(isect->dPdt, &obj->object_to_world);

	isect->object = obj;

	return 1;
}

/* ObjectGroup interfaces */
struct ObjectGroup *ObjGroupNew(void)
{
#if 0
	struct ObjectGroup *grp;

	grp = (struct ObjectGroup *) malloc(sizeof(struct ObjectGroup));
	if (grp == NULL)
		return NULL;

	grp->accelerator = AccNew(ACC_BVH);
	if (grp->accelerator == NULL) {
		ObjGroupFree(grp);
		return NULL;
	}

	grp->objects = ArrNew(sizeof(struct ObjectInstance *));
	if (grp->objects == NULL) {
		ObjGroupFree(grp);
		return NULL;
	}

	BOX3_SET(grp->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	return grp;
#endif
	struct ObjectGroup *grp;

	grp = (struct ObjectGroup *) malloc(sizeof(struct ObjectGroup));
	if (grp == NULL)
		return NULL;

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

	return grp;
}

void ObjGroupFree(struct ObjectGroup *grp)
{
#if 0
	if (grp == NULL)
		return;

	if (grp->accelerator != NULL)
		AccFree(grp->accelerator);

	if (grp->objects != NULL)
		ArrFree(grp->objects);

	free(grp);
#endif
	if (grp == NULL)
		return;

	obj_list_free(grp->surface_list);
	obj_list_free(grp->volume_list);
	free(grp);
}

void ObjGroupAdd(struct ObjectGroup *grp, const struct ObjectInstance *obj)
{
#if 0
	ArrPushPointer(grp->objects, obj);
	BoxAddBox(grp->bounds, obj->bounds);
	update_group_accelerator(grp);
#endif
	int primtype;
	primtype = AccGetPrimitiveType(obj->acc);

	if (primtype == ACC_PRIM_SURFACE) {
		obj_list_add(grp->surface_list, obj);
	}
	else if (primtype == ACC_PRIM_VOLUME) {
		obj_list_add(grp->volume_list, obj);
	}
	else {
		printf("fatal error: bad primtype: %d\n", primtype);
		abort();
	}
}

#if 0
const struct Accelerator *ObjGroupGetAccelerator(const struct ObjectGroup *grp)
{
	return grp->accelerator;
	return grp->surface_list->accelerator;
}
#endif

const struct Accelerator *ObjGroupGetSurfaceAccelerator(const struct ObjectGroup *grp)
{
	return grp->surface_list->accelerator;
}

const struct Accelerator *ObjGroupGetVolumeAccelerator(const struct ObjectGroup *grp)
{
	return grp->volume_list->accelerator;
}

static void update_matrix_and_bounds(struct ObjectInstance *obj)
{
	ComputeMatrix(obj->xform_order, obj->rotate_order,
			obj->translate[0], obj->translate[1], obj->translate[2],
			obj->rotate[0],    obj->rotate[1],    obj->rotate[2],
			obj->scale[0],     obj->scale[1],     obj->scale[2],
			&obj->object_to_world);

	MatInverse(&obj->world_to_object, &obj->object_to_world);
	update_object_bounds(obj);
}

static void update_object_bounds(struct ObjectInstance *obj)
{
	AccGetBounds(obj->acc, obj->bounds);
	TransformBounds(obj->bounds, &obj->object_to_world);
}

static void object_bounds(const void *prim_set, int prim_id, double *bounds)
{
	const struct ObjectInstance **objects = (const struct ObjectInstance **) prim_set;
	BOX3_COPY(bounds, objects[prim_id]->bounds);
}

static int object_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	const struct ObjectInstance **objects = (const struct ObjectInstance **) prim_set;
	return ObjIntersect(objects[prim_id], ray, isect, t_hit);
}

#if 0
static void update_group_accelerator(struct ObjectGroup *grp)
{
	assert(grp != NULL);
	assert(grp->accelerator != NULL);
	assert(grp->objects != NULL);

	AccSetTargetGeometry(grp->accelerator,
			ACC_PRIM_SURFACE,
			grp->objects->data,
			grp->objects->nelems,
			grp->bounds,
			object_ray_intersect,
			object_bounds);
}
#endif

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
	ArrPushPointer(list->objects, obj);
	BoxAddBox(list->bounds, obj->bounds);

	AccSetTargetGeometry(list->accelerator,
			ACC_PRIM_SURFACE,
			list->objects->data,
			list->objects->nelems,
			list->bounds,
			object_ray_intersect,
			object_bounds);
}

