/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Camera.h"
#include "Property.h"
#include "Numeric.h"
#include "Memory.h"
#include "Vector.h"
#include "Ray.h"
#include <string.h>
#include <assert.h>

struct Camera {
	struct TransformSampleList transform_samples;

	double aspect;
	double znear;
	double zfar;
	double fov;

	struct Vector2 uv_size;
};

static void compute_uv_size(struct Camera *cam);
static void compute_ray_target(const struct Camera *cam,
		const struct Vector2 *uv, struct Vector *target);

struct Camera *CamNew(const char *type)
{
	struct Camera *cam = MEM_ALLOC(struct Camera);
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
	MEM_FREE(cam);
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

void CamGetRay(const struct Camera *cam, const struct Vector2 *screen_uv,
		double time, struct Ray *ray)
{
	struct Transform transform_interp;
	struct Vector target = {0, 0, 0};
	struct Vector eye = {0, 0, 0};

	XfmLerpTransformSample(&cam->transform_samples, time, &transform_interp);

	compute_ray_target(cam, screen_uv, &target);
	XfmTransformPoint(&transform_interp, &target);
	XfmTransformPoint(&transform_interp, &eye);

	ray->dir.x = target.x - eye.x;
	ray->dir.y = target.y - eye.y;
	ray->dir.z = target.z - eye.z;

	VEC3_NORMALIZE(&ray->dir);
	ray->orig = eye;

	ray->tmin = cam->znear;
	ray->tmax = cam->zfar;
}

static void compute_uv_size(struct Camera *cam)
{
	cam->uv_size.y = 2 * tan(RADIAN(cam->fov/2));
	cam->uv_size.x = cam->uv_size.y * cam->aspect;
}

static void compute_ray_target(const struct Camera *cam,
		const struct Vector2 *uv, struct Vector *target)
{
	target->x = (uv->x - .5) * cam->uv_size.x;
	target->y = (uv->y - .5) * cam->uv_size.y;
	target->z = -1;
}

