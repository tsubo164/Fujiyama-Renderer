// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_CAMERA_H
#define FJ_CAMERA_H

#include "fj_transform.h"
#include "fj_vector.h"
#include "fj_types.h"

namespace fj {

class Ray;

class Camera {
public:
  Camera();
  ~Camera();

  void SetAspect(Real aspect);
  void SetFov(Real fov);
  void SetNearPlane(Real znear);
  void SetFarPlane(Real zfar);

  void SetTranslate(Real tx, Real ty, Real tz, Real time);
  void SetRotate(Real rx, Real ry, Real rz, Real time);
  void SetTransformOrder(int order);
  void SetRotateOrder(int order);

  void GetRay(const Vector2 &screen_uv, Real time, Ray *ray) const;

private:
  void compute_uv_size();
  Vector compute_ray_target(const Vector2 &uv) const;

  TransformSampleList transform_samples_;

  Real aspect_;
  Real znear_;
  Real zfar_;
  Real fov_;

  Vector2 uv_size_;
};

} // namespace xxx

#endif // FJ_XXX_H
