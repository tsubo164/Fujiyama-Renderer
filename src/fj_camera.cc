// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_camera.h"
#include "fj_property.h"
#include "fj_numeric.h"
#include "fj_ray.h"

#include <cassert>
#include <cmath>

namespace fj {

Camera::Camera() :
  transform_samples_(),
  aspect_(1),
  znear_(.01),
  zfar_(1000),
  fov_(30.),
  uv_size_()
{
  XfmInitTransformSampleList(&transform_samples_);
  compute_uv_size();
}

Camera::~Camera()
{
}

void Camera::SetAspect(Real aspect)
{
  assert(aspect > 0);
  aspect_ = aspect;

  compute_uv_size();
}

void Camera::SetFov(Real fov)
{
  assert(fov > 0);
  assert(fov < 180);
  fov_ = fov;

  compute_uv_size();
}

void Camera::SetNearPlane(Real znear)
{
  assert(znear > 0);
  znear_ = znear;
}

void Camera::SetFarPlane(Real zfar)
{
  assert(zfar > 0);
  zfar_ = zfar;
}

void Camera::SetTranslate(Real tx, Real ty, Real tz, Real time)
{
  XfmPushTranslateSample(&transform_samples_, tx, ty, tz, time);
}

void Camera::SetRotate(Real rx, Real ry, Real rz, Real time)
{
  XfmPushRotateSample(&transform_samples_, rx, ry, rz, time);
}

void Camera::SetTransformOrder(int order)
{
  XfmSetSampleTransformOrder(&transform_samples_, order);
}

void Camera::SetRotateOrder(int order)
{
  XfmSetSampleRotateOrder(&transform_samples_, order);
}

void Camera::GetRay(const Vector2 &screen_uv, Real time, Ray *ray) const
{
  Transform transform_interp;
  XfmLerpTransformSample(&transform_samples_, time, &transform_interp);

  Vector target = compute_ray_target(screen_uv);
  Vector eye;

  XfmTransformPoint(&transform_interp, &target);
  XfmTransformPoint(&transform_interp, &eye);

  ray->dir  = target - eye;
  Normalize(&ray->dir);
  ray->orig = eye;

  ray->tmin = znear_;
  ray->tmax = zfar_;
}

void Camera::compute_uv_size()
{
  uv_size_[1] = 2 * tan(Radian(fov_ / 2.));
  uv_size_[0] = uv_size_[1] * aspect_;
}

Vector Camera::compute_ray_target(const Vector2 &uv) const
{
  return Vector(
      (uv[0] - .5) * uv_size_[0],
      (uv[1] - .5) * uv_size_[1],
      -1);
}

} // namespace xxx
