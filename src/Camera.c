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

struct Camera {
	double center[3];
	double dir[3];
	double up[3];

	double scrn_bottom_left[3];

	double uvec[3];
	double vvec[3];
	double wvec[3];

	double aspect;
	double znear;
	double zfar;
	double fov;
};

static void compute_uvw_vectors(struct Camera *cam);

struct Camera *CamNew(const char *type)
{
	struct Camera *cam = (struct Camera *) malloc(sizeof(struct Camera));
	if (cam == NULL)
		return NULL;

	VEC3_SET(cam->center, 0, 0, 0);
	VEC3_SET(cam->dir, 0, 0, -1);
	VEC3_SET(cam->up, 0, 1, 0);

	cam->aspect = 1;
	cam->fov = 30;
	cam->znear = .01;
	cam->zfar = 1000;

	compute_uvw_vectors(cam);

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
	compute_uvw_vectors(cam);
}

void CamSetFov(struct Camera *cam, double fov)
{
	assert(fov > 0);
	assert(fov < 180);
	cam->fov = fov;
	compute_uvw_vectors(cam);
}

void CamSetNearPlane(struct Camera *cam, double znear)
{
	assert(znear > 0);
	cam->znear = znear;
	compute_uvw_vectors(cam);
}

void CamSetFarPlane(struct Camera *cam, double zfar)
{
	assert(zfar > 0);
	cam->zfar = zfar;
	compute_uvw_vectors(cam);
}

void CamSetPosition(struct Camera *cam, double xpos, double ypos, double zpos)
{
	VEC3_SET(cam->center, xpos, ypos, zpos);
	compute_uvw_vectors(cam);
}

void CamSetDirection(struct Camera *cam, double xdir, double ydir, double zdir)
{
	VEC3_SET(cam->dir, xdir, ydir, zdir);
	compute_uvw_vectors(cam);
}

void CamGetRay(const struct Camera *cam, const double *screen_uv, struct Ray *ray)
{
	int i;
	for (i = 0; i < 3; i++) {
		ray->dir[i] =
				cam->scrn_bottom_left[i] +
				screen_uv[0] * cam->uvec[i] +
				screen_uv[1] * cam->vvec[i] -
				cam->center[i];
	}
	VEC3_NORMALIZE(ray->dir);
	VEC3_COPY(ray->orig, cam->center);
	ray->tmin = cam->znear;
	ray->tmax = cam->zfar;
}

static void compute_uvw_vectors(struct Camera *cam)
{
	double halfu, halfv;
	int i;

	/* assumes dir and up are normalized */
	VEC3_COPY(cam->wvec, cam->dir);
	VEC3_NORMALIZE(cam->wvec);

	VEC3_CROSS(cam->uvec, cam->wvec, cam->up);
	VEC3_NORMALIZE(cam->uvec);

	VEC3_CROSS(cam->vvec, cam->uvec, cam->wvec);
	VEC3_NORMALIZE(cam->vvec);

	halfv = tan(RADIAN(cam->fov/2));
	halfu = halfv * cam->aspect;

	for (i = 0; i < 3; i++) {
		cam->scrn_bottom_left[i] = cam->center[i] +
				cam->wvec[i] -
				halfu * cam->uvec[i] -
				halfv * cam->vvec[i];
	}

	VEC3_MUL_ASGN(cam->uvec, 2 * halfu);
	VEC3_MUL_ASGN(cam->vvec, 2 * halfv);
}

