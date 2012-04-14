/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "ObjectInstance.h"
#include "Intersection.h"
#include "Accelerator.h"
#include "Vector.h"
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
	const struct Volume *volume;
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

static void update_matrix_and_bounds(struct ObjectInstance *obj);
static void update_object_bounds(struct ObjectInstance *obj);

/* TODO remove acc argument */
/* ObjectInstance interfaces */
struct ObjectInstance *ObjNew(const struct Accelerator *acc)
{
	struct ObjectInstance *obj;

	obj = (struct ObjectInstance *) malloc(sizeof(struct ObjectInstance));
	if (obj == NULL)
		return NULL;

	obj->acc = acc;
	obj->volume = NULL;

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

int ObjSetVolume(struct ObjectInstance *obj, const struct Volume *volume)
{
	if (obj->acc != NULL)
		return -1;

	if (obj->volume != NULL)
		return -1;

	obj->volume = volume;
	update_matrix_and_bounds(obj);

	assert(obj->acc == NULL && obj->volume != NULL);
	return 0;
}

int ObjIsSurface(const struct ObjectInstance *obj)
{
	if (obj->acc == NULL)
		return 0;

	assert(obj->volume == NULL);
	return 1;
}

int ObjIsVolume(const struct ObjectInstance *obj)
{
	if (obj->volume == NULL)
		return 0;

	assert(obj->acc == NULL);
	return 1;
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

void ObjGetBounds(const struct ObjectInstance *obj, double *bounds)
{
	BOX3_COPY(bounds, obj->bounds);
}

int ObjIntersect(const struct ObjectInstance *obj, const struct Ray *ray,
		struct Intersection *isect)
{
	struct Ray ray_in_objspace;
	int hit;

	if (!ObjIsSurface(obj))
		return 0;

	/* transform ray to object space */
	ray_in_objspace = *ray;
	TransformPoint(ray_in_objspace.orig, &obj->world_to_object);
	TransformVector(ray_in_objspace.dir, &obj->world_to_object);

	hit = AccIntersect(obj->acc, &ray_in_objspace, isect);
	if (!hit)
		return 0;

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
	if (ObjIsSurface(obj)) {
		AccGetBounds(obj->acc, obj->bounds);
	}
	else if (ObjIsVolume(obj)) {
		VolGetBounds(obj->volume, obj->bounds);
	}
	else {
		/* TODO TEST allowing neither surface nor volume */
		/*
		printf("fatal error: object is neither surface nor volume\n");
		abort();
		*/
		BOX3_SET(obj->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	}
	TransformBounds(obj->bounds, &obj->object_to_world);
}

int ObjGetVolumeSample(const struct ObjectInstance *obj, const double *point,
			struct VolumeSample *sample)
{
	int hit;
	double point_in_objspace[3];

	if (!ObjIsVolume(obj))
		return 0;

	VEC3_COPY(point_in_objspace, point);
	TransformPoint(point_in_objspace, &obj->world_to_object);

	hit = VolGetSample(obj->volume, point_in_objspace, sample);

	return hit;
}

