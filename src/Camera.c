/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Camera.h"
#include "Property.h"
#include "Numeric.h"
#include "Vector.h"
#include "Ray.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
#if 0
	CamSetTransformOrder(cam, ORDER_SRT);
	CamSetRotateOrder(cam, ORDER_ZXY);
#endif
	/* need to both orders before calling XfmSetTransform */
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

	struct PropertySample T = INIT_PROPERTYSAMPLE;
	struct PropertySample R = INIT_PROPERTYSAMPLE;

	PropLerpSamples(&cam->translate_list, time, &T);
	PropLerpSamples(&cam->rotate_list, time, &R);

	XfmSetTransform(&transform,
		cam->transform_order, cam->rotate_order,
		T.vector[0], T.vector[1], T.vector[2],
		R.vector[0], R.vector[1], R.vector[2],
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

