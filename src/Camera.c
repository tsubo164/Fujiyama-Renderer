/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
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
	struct TransformSampleList transform_samples;

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

	XfmInitTransformSampleList(&cam->transform_samples);

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
	XfmPushTranslateSample(&cam->transform_samples, tx, ty, tz, time);
}

void CamSetRotate(struct Camera *cam, double rx, double ry, double rz, double time)
{
	XfmPushRotateSample(&cam->transform_samples, rx, ry, rz, time);
}

void CamSetTransformOrder(struct Camera *cam, int order)
{
	XfmSetSampleTransformOrder(&cam->transform_samples, order);
}

void CamSetRotateOrder(struct Camera *cam, int order)
{
	XfmSetSampleRotateOrder(&cam->transform_samples, order);
}

void CamGetRay(const struct Camera *cam, const double *screen_uv,
		double time, struct Ray *ray)
{
	struct Transform transform_interp;
	double target[3] = {0, 0, 0};
	double eye[3] = {0, 0, 0};

	XfmLerpTransformSample(&cam->transform_samples, time, &transform_interp);

	compute_ray_target(cam, screen_uv, target);
	XfmTransformPoint(&transform_interp, target);
	XfmTransformPoint(&transform_interp, eye);

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

