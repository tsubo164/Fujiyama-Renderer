/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Camera.h"
#include "Numeric.h"
#include "Vector.h"
#include "Ray.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* TODO TEST PropertySample */
#define MAX_PROPERTY_SAMPLES 8

struct PropertySample {
	double vector[4];
	double time;
};

struct PropertySampleList {
	struct PropertySample samples[MAX_PROPERTY_SAMPLES];
	int sample_count;
};

void PropInitSampleList(struct PropertySampleList *list)
{
	const struct PropertySample initial_value = {{0, 0, 0}, 0};
	int i;

	for (i = 0; i < MAX_PROPERTY_SAMPLES; i++) {
		list->samples[i] = initial_value;
	}

	list->sample_count = 1;
}

static int compare_property_sample(const void *sample0, const void *sample1)
{
	const double x = ((struct PropertySample *) sample0)->time;
	const double y = ((struct PropertySample *) sample1)->time;

	if (x > y)
		return 1;
	else if (x < y)
		return -1;
	else
		return 0;
}

int PropPushSample(struct PropertySampleList *list, const struct PropertySample *sample)
{
	int next_index = 0;
	int i;

	if (list->sample_count >= MAX_PROPERTY_SAMPLES) {
		return -1;
	}

	for (i = 0; i < list->sample_count; i++) {
		if (list->samples[i].time == sample->time) {
			list->samples[i] = *sample;
			return 0;
		}
	}

	next_index = list->sample_count;
	list->samples[next_index] = *sample;
	list->sample_count++;

	qsort(list->samples, list->sample_count,
			sizeof(struct PropertySample),
			compare_property_sample);

	return 0;
}

void PropLerpSamples(const struct PropertySampleList *list, double time, double *dst)
{
	int i;

	if (list->samples[0].time >= time) {
		VEC4_COPY(dst, list->samples[0].vector);
		return;
	}

	if (list->samples[list->sample_count-1].time <= time) {
		VEC4_COPY(dst, list->samples[list->sample_count-1].vector);
		return;
	}

	for (i = 0; i < list->sample_count; i++) {
		if (list->samples[i].time == time) {
			VEC4_COPY(dst, list->samples[i].vector);
			return;
		}
		if (list->samples[i].time > time) {
			const struct PropertySample *sample0 = &list->samples[i-1];
			const struct PropertySample *sample1 = &list->samples[i];
			const double t = Fit(time, sample0->time, sample1->time, 0, 1);
			VEC4_LERP(dst, sample0->vector, sample1->vector, t);
			return;
		}
	}
}

struct Camera {
	struct PropertySampleList translate_list;
	struct PropertySampleList rotate_list;
	int transform_order;
	int rotate_order;

	double aspect;
	double znear;
	double zfar;
	double fov;

	double uv_size[2];
};

static void compute_uv_size(struct Camera *cam);
static void compute_ray_target(const struct Camera *cam, const double *uv, double *target);

struct Camera *CamNew(const char *type)
{
	struct Camera *cam = (struct Camera *) malloc(sizeof(struct Camera));
	if (cam == NULL)
		return NULL;

	PropInitSampleList(&cam->translate_list);
	PropInitSampleList(&cam->rotate_list);
	cam->transform_order = ORDER_SRT;
	cam->rotate_order = ORDER_ZXY;

	CamSetAspect(cam, 1);
	CamSetFov(cam, 30);
	CamSetNearPlane(cam, .01);
	CamSetFarPlane(cam, 1000);

	compute_uv_size(cam);

	return cam;
}

void CamFree(struct Camera *cam)
{
	if (cam == NULL)
		return;
	free(cam);
}

void CamSetAspect(struct Camera *cam, double aspect)
{
	assert(aspect > 0);
	cam->aspect = aspect;

	compute_uv_size(cam);
}

void CamSetFov(struct Camera *cam, double fov)
{
	assert(fov > 0);
	assert(fov < 180);
	cam->fov = fov;

	compute_uv_size(cam);
}

void CamSetNearPlane(struct Camera *cam, double znear)
{
	assert(znear > 0);
	cam->znear = znear;
}

void CamSetFarPlane(struct Camera *cam, double zfar)
{
	assert(zfar > 0);
	cam->zfar = zfar;
}

/* TODO remove CamSetPosition */
void CamSetPosition(struct Camera *cam, double xpos, double ypos, double zpos)
{
}

/* TODO remove CamSetDirection */
void CamSetDirection(struct Camera *cam, double xdir, double ydir, double zdir)
{
}

void CamSetTranslate(struct Camera *cam, double tx, double ty, double tz, double time)
{
	struct PropertySample sample = {{0, 0, 0}, 0};

	sample.vector[0] = tx;
	sample.vector[1] = ty;
	sample.vector[2] = tz;
	sample.time = time;

	PropPushSample(&cam->translate_list, &sample);
}

void CamSetRotate(struct Camera *cam, double rx, double ry, double rz, double time)
{
	struct PropertySample sample = {{0, 0, 0}, 0};

	sample.vector[0] = rx;
	sample.vector[1] = ry;
	sample.vector[2] = rz;
	sample.time = time;

	PropPushSample(&cam->rotate_list, &sample);
}

void CamSetTransformOrder(struct Camera *cam, int order)
{
	if (!XfmIsTransformOrder(order)) {
		return;
	}
	cam->transform_order = order;
}

void CamSetRotateOrder(struct Camera *cam, int order)
{
	if (!XfmIsRotateOrder(order)) {
		return;
	}
	cam->rotate_order = order;
}

void CamGetRay(const struct Camera *cam, const double *screen_uv, double time,
		struct Ray *ray)
{
	double target[3] = {0, 0, 0};
	double eye[3] = {0, 0, 0};
	struct Transform transform;

	double T[3] = {0};
	double R[3] = {0};

	PropLerpSamples(&cam->translate_list, time, T);
	PropLerpSamples(&cam->rotate_list, time, R);

	XfmSetTransform(&transform,
		ORDER_SRT, ORDER_ZXY,
		T[0], T[1], T[2],
		R[0], R[1], R[2],
		1, 1, 1);

	compute_ray_target(cam, screen_uv, target);
	XfmTransformPoint(&transform, target);
	XfmTransformPoint(&transform, eye);

	VEC3_SUB(ray->dir, target, eye);

	VEC3_NORMALIZE(ray->dir);
	VEC3_COPY(ray->orig, eye);
	ray->tmin = cam->znear;
	ray->tmax = cam->zfar;
}

static void compute_uv_size(struct Camera *cam)
{
	cam->uv_size[1] = 2 * tan(RADIAN(cam->fov/2));
	cam->uv_size[0] = cam->uv_size[1] * cam->aspect;
}

static void compute_ray_target(const struct Camera *cam, const double *uv, double *target)
{
	target[0] = (uv[0] - .5) * cam->uv_size[0];
	target[1] = (uv[1] - .5) * cam->uv_size[1];
	target[2] = -1;
}

