/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "ObjectInstance.h"
#include "Intersection.h"
#include "Accelerator.h"
#include "Interval.h"
#include "Property.h"
#include "Numeric.h"
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

	/* transformation properties */
	struct Transform transform;
	struct Transform transform_sample;
	/* TODO TEST */
	struct PropertySampleList translate_list;
	struct PropertySampleList rotate_list;
	struct PropertySampleList scale_list;
	int transform_order;
	int rotate_order;

	/* non-geometric properties */
	const struct Shader *shader;
	const struct Light **target_lights;
	int n_target_lights;
	const struct ObjectGroup *reflection_target;
	const struct ObjectGroup *refraction_target;
};

static void update_object_bounds(struct ObjectInstance *obj);
static void merge_bounds_samples(struct ObjectInstance *obj);

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

	XfmReset(&obj->transform);
	update_object_bounds(obj);

	/* TODO TEST */
	PropInitSampleList(&obj->translate_list);
	PropInitSampleList(&obj->rotate_list);
	PropInitSampleList(&obj->scale_list);
	/* TODO need to both orders before calling XfmSetTransform */
	obj->transform_order = ORDER_SRT;
	obj->rotate_order = ORDER_ZXY;
	ObjSetScale(obj, 1, 1, 1, 0);

	obj->shader = NULL;
	obj->target_lights = NULL;
	obj->n_target_lights = 0;

	obj->reflection_target = NULL;
	obj->refraction_target = NULL;

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
	update_object_bounds(obj);

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

void ObjSetTranslate(struct ObjectInstance *obj, double tx, double ty, double tz, double time)
{
	/* TODO TEST */
	struct PropertySample sample = {{0, 0, 0}, 0};

	sample.vector[0] = tx;
	sample.vector[1] = ty;
	sample.vector[2] = tz;
	sample.time = time;

	PropPushSample(&obj->translate_list, &sample);

	XfmSetTranslate(&obj->transform, tx, ty, tz);
	update_object_bounds(obj);
}

void ObjSetRotate(struct ObjectInstance *obj, double rx, double ry, double rz, double time)
{
	/* TODO TEST */
	struct PropertySample sample = {{0, 0, 0}, 0};

	sample.vector[0] = rx;
	sample.vector[1] = ry;
	sample.vector[2] = rz;
	sample.time = time;

	PropPushSample(&obj->rotate_list, &sample);

	XfmSetRotate(&obj->transform, rx, ry, rz);
	update_object_bounds(obj);
}

void ObjSetScale(struct ObjectInstance *obj, double sx, double sy, double sz, double time)
{
	/* TODO TEST */
	struct PropertySample sample = {{0, 0, 0}, 0};

	sample.vector[0] = sx;
	sample.vector[1] = sy;
	sample.vector[2] = sz;
	sample.time = time;

	PropPushSample(&obj->scale_list, &sample);

	XfmSetScale(&obj->transform, sx, sy, sz);
	update_object_bounds(obj);
}

void ObjSetTransformOrder(struct ObjectInstance *obj, int order)
{
	XfmSetTransformOrder(&obj->transform, order);
	update_object_bounds(obj);
}

void ObjSetRotateOrder(struct ObjectInstance *obj, int order)
{
	XfmSetRotateOrder(&obj->transform, order);
	update_object_bounds(obj);
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
	struct Ray ray_object_space;
	int hit = 0;
	double time = 0;

/* ====================== */
	struct PropertySample T = INIT_PROPERTYSAMPLE;
	struct PropertySample R = INIT_PROPERTYSAMPLE;
	struct PropertySample S = INIT_PROPERTYSAMPLE;

	PropLerpSamples(&obj->translate_list, time, &T);
	PropLerpSamples(&obj->rotate_list, time, &R);
	PropLerpSamples(&obj->scale_list, time, &S);

	XfmSetTransform((struct Transform *) &obj->transform_sample,
		obj->transform_order, obj->rotate_order,
		T.vector[0], T.vector[1], T.vector[2],
		R.vector[0], R.vector[1], R.vector[2],
		S.vector[0], S.vector[1], S.vector[2]);
/* ====================== */

	if (!ObjIsSurface(obj))
		return 0;

	/* transform ray to object space */
	ray_object_space = *ray;
	XfmTransformPointInverse(&obj->transform_sample, ray_object_space.orig);
	XfmTransformVectorInverse(&obj->transform_sample, ray_object_space.dir);

	hit = AccIntersect(obj->acc, &ray_object_space, isect);
	if (!hit)
		return 0;

	/* transform intersection back to world space */
	XfmTransformPoint(&obj->transform_sample, isect->P);
	XfmTransformVector(&obj->transform_sample, isect->N);
	VEC3_NORMALIZE(isect->N);

	/* TODO should make TransformLocalGeometry? */
	XfmTransformVector(&obj->transform_sample, isect->dPds);
	XfmTransformVector(&obj->transform_sample, isect->dPdt);

	isect->object = obj;

	return 1;
#if 0
	struct Ray ray_object_space;
	int hit = 0;

	if (!ObjIsSurface(obj))
		return 0;

	/* transform ray to object space */
	ray_object_space = *ray;
	XfmTransformPointInverse(&obj->transform, ray_object_space.orig);
	XfmTransformVectorInverse(&obj->transform, ray_object_space.dir);

	hit = AccIntersect(obj->acc, &ray_object_space, isect);
	if (!hit)
		return 0;

	/* transform intersection back to world space */
	XfmTransformPoint(&obj->transform, isect->P);
	XfmTransformVector(&obj->transform, isect->N);
	VEC3_NORMALIZE(isect->N);

	/* TODO should make TransformLocalGeometry? */
	XfmTransformVector(&obj->transform, isect->dPds);
	XfmTransformVector(&obj->transform, isect->dPdt);

	isect->object = obj;

	return 1;
#endif
}

int ObjVolumeIntersect(const struct ObjectInstance *obj, const struct Ray *ray,
			struct Interval *interval)
{
	struct Ray ray_object_space;
	double volume_bounds[6] = {0};
	double boxhit_tmin = 0;
	double boxhit_tmax = 0;
	int hit = 0;

	if (!ObjIsVolume(obj))
		return 0;

	VolGetBounds(obj->volume, volume_bounds);

	/* transform ray to object space */
	ray_object_space = *ray;
	XfmTransformPointInverse(&obj->transform, ray_object_space.orig);
	XfmTransformVectorInverse(&obj->transform, ray_object_space.dir);

	hit = BoxRayIntersect(volume_bounds,
			ray_object_space.orig,
			ray_object_space.dir,
			ray_object_space.tmin,
			ray_object_space.tmax,
			&boxhit_tmin, &boxhit_tmax);

	if (!hit) {
		return 0;
	}

	interval->tmin = boxhit_tmin;
	interval->tmax = boxhit_tmax;
	interval->object = obj;

	return 1;
}

int ObjGetVolumeSample(const struct ObjectInstance *obj, const double *point,
			struct VolumeSample *sample)
{
	int hit;
	double point_in_objspace[3];

	if (!ObjIsVolume(obj))
		return 0;

	VEC3_COPY(point_in_objspace, point);
	XfmTransformPointInverse(&obj->transform, point_in_objspace);

	hit = VolGetSample(obj->volume, point_in_objspace, sample);

	return hit;
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
		/* TODO TEST allowing state where neither surface nor volume */
		/*
		printf("fatal error: object is neither surface nor volume\n");
		abort();
		*/
		BOX3_SET(obj->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	}
	XfmTransformBounds(&obj->transform, obj->bounds);

	{
		/* TODO TEST allowing neither surface nor volume */
		merge_bounds_samples(obj);
		/*
	BoxPrint(obj->bounds);
		*/
	}
}

static void merge_bounds_samples(struct ObjectInstance *obj)
{
	double merged_bounds[6] = {FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
	double original_bounds[6] = {FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
	double S_max[3] = {1, 1, 1};
	double R_max[3] = {0, 0, 0};
	int i;

	VEC3_COPY(S_max, obj->scale_list.samples[0].vector);
	if (obj->rotate_list.sample_count == 1) {
		VEC3_COPY(R_max, obj->rotate_list.samples[0].vector);
	} else {
		double b[6] = {0};
		double x, y, z, d;
		AccGetBounds(obj->acc, b);
		x = BOX3_XSIZE(b);
		y = BOX3_YSIZE(b);
		z = BOX3_ZSIZE(b);
	}
	/*
		printf("============= (%g, %g, %g)\n", S_max[0], S_max[1], S_max[2]);
	*/
	for (i = 1; i < obj->scale_list.sample_count; i++) {
		S_max[0] = MAX(S_max[0], obj->scale_list.samples[i].vector[0]);
		S_max[1] = MAX(S_max[1], obj->scale_list.samples[i].vector[1]);
		S_max[2] = MAX(S_max[2], obj->scale_list.samples[i].vector[2]);
	}

	AccGetBounds(obj->acc, original_bounds);

	for (i = 0; i < obj->translate_list.sample_count; i++) {
		double sample_bounds[6] = {FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
		const double *T = obj->translate_list.samples[i].vector;
		struct Transform transform;

		XfmSetTransform(&transform,
			obj->transform_order, obj->rotate_order,
			T[0], T[1], T[2],
			R_max[0], R_max[1], R_max[2],
			S_max[0], S_max[1], S_max[2]);
		/*
			0, 0, 0,
		*/
		/*
			1, 1, 1);
		*/

		/*
		ObjGetBounds(obj, original_bounds);
		*/
		BOX3_COPY(sample_bounds, original_bounds);
		XfmTransformBounds(&transform, sample_bounds);
		BoxAddBox(merged_bounds, sample_bounds);
	}

	BOX3_COPY(obj->bounds, merged_bounds);
	BoxPrint(obj->bounds);
	/*
	*/
}

