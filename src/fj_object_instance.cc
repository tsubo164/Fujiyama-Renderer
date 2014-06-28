// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_object_instance.h"
#include "fj_intersection.h"
#include "fj_accelerator.h"
#include "fj_object_group.h"
#include "fj_interval.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_volume.h"
#include "fj_matrix.h"
#include "fj_ray.h"

#include <cassert>

namespace fj {

ObjectInstance::ObjectInstance() :
    acc_(NULL),
    volume_(NULL),
    bounds_(),

    transform_samples_(),

    shader_(NULL),
    target_lights_(NULL),
    n_target_lights_(0),

    reflection_target_(NULL),
    refraction_target_(NULL),
    shadow_target_(NULL),
    self_target_(NULL)
{
  XfmInitTransformSampleList(&transform_samples_);
  update_bounds();
}

ObjectInstance::~ObjectInstance()
{
}

int ObjectInstance::SetSurface(const Accelerator *acc)
{
  if (acc_ != NULL)
    return -1;

  if (volume_ != NULL)
    return -1;

  acc_ = acc;
  update_bounds();

  assert(acc_ != NULL && volume_ == NULL);
  return 0;
}

int ObjectInstance::SetVolume(const Volume *volume)
{
  if (acc_ != NULL)
    return -1;

  if (volume_ != NULL)
    return -1;

  volume_ = volume;
  update_bounds();

  assert(acc_ == NULL && volume_ != NULL);
  return 0;
}

bool ObjectInstance::IsSurface() const
{
  if (acc_ == NULL)
    return false;

  assert(volume_ == NULL);
  return true;
}

bool ObjectInstance::IsVolume() const
{
  if (volume_ == NULL)
    return false;

  assert(acc_ == NULL);
  return true;
}

void ObjectInstance::SetTranslate(Real tx, Real ty, Real tz, Real time)
{
  XfmPushTranslateSample(&transform_samples_, tx, ty, tz, time);
  update_bounds();
}

void ObjectInstance::SetRotate(Real rx, Real ry, Real rz, Real time)
{
  XfmPushRotateSample(&transform_samples_, rx, ry, rz, time);
  update_bounds();
}

void ObjectInstance::SetScale(Real sx, Real sy, Real sz, Real time)
{
  XfmPushScaleSample(&transform_samples_, sx, sy, sz, time);
  update_bounds();
}

void ObjectInstance::SetTransformOrder(int order)
{
  XfmSetSampleTransformOrder(&transform_samples_, order);
  update_bounds();
}

void ObjectInstance::SetRotateOrder(int order)
{
  XfmSetSampleRotateOrder(&transform_samples_, order);
  update_bounds();
}

void ObjectInstance::SetShader(const Shader *shader)
{
  shader_ = shader;
}

void ObjectInstance::SetLightList(const Light **lights, int count)
{
  target_lights_ = lights;
  n_target_lights_ = count;
}

void ObjectInstance::SetReflectTarget(const ObjectGroup *group)
{
  reflection_target_ = group;
}

void ObjectInstance::SetRefractTarget(const ObjectGroup *group)
{
  refraction_target_ = group;
}

void ObjectInstance::SetShadowTarget(const ObjectGroup *group)
{
  shadow_target_ = group;
}

// TODO come up with better way to make self target
void ObjectInstance::SetSelfHitTarget(const ObjectGroup *group)
{
  self_target_ = group;
}

const ObjectGroup *ObjectInstance::GetReflectTarget() const
{
  return reflection_target_;
}

const ObjectGroup *ObjectInstance::GetRefractTarget() const
{
  return refraction_target_;
}

const ObjectGroup *ObjectInstance::GetShadowTarget() const
{
  return shadow_target_;
}

const ObjectGroup *ObjectInstance::GetSelfHitTarget() const
{
  return self_target_;
}

const Shader *ObjectInstance::GetShader() const
{
  return shader_;
}

const Light **ObjectInstance::GetLightList() const
{
  return target_lights_;
}

int ObjectInstance::GetLightCount() const
{
  return n_target_lights_;
}

const Box &ObjectInstance::GetBounds() const
{
  return bounds_;
}

void ObjectInstance::ComputeBounds()
{
  update_bounds();
}

bool ObjectInstance::RayIntersect(const Ray &ray, Real time, Intersection *isect) const
{
  if (!IsSurface()) {
    return false;
  }

  Transform transform_interp;
  XfmLerpTransformSample(&transform_samples_, time, &transform_interp);

  // transform ray to object space
  Ray ray_object_space = ray;
  XfmTransformPointInverse(&transform_interp, &ray_object_space.orig);
  XfmTransformVectorInverse(&transform_interp, &ray_object_space.dir);

  const bool hit = acc_->Intersect(ray_object_space, time, isect);
  if (!hit) {
    return false;
  }

  // transform intersection back to world space
  XfmTransformPoint(&transform_interp, &isect->P);
  XfmTransformVector(&transform_interp, &isect->N);
  Normalize(&isect->N);

  XfmTransformVector(&transform_interp, &isect->dPdu);
  XfmTransformVector(&transform_interp, &isect->dPdv);

  isect->object = this;

  return true;
}

bool ObjectInstance::RayVolumeIntersect(const Ray &ray, Real time,
    Interval *interval) const
{
  if (!IsVolume()) {
    return false;
  }

  Transform transform_interp;
  XfmLerpTransformSample(&transform_samples_, time, &transform_interp);

  // transform ray to object space
  Ray ray_object_space = ray;
  XfmTransformPointInverse(&transform_interp, &ray_object_space.orig);
  XfmTransformVectorInverse(&transform_interp, &ray_object_space.dir);

  const Box volume_bounds = volume_->GetBounds();
  Real boxhit_tmin = 0;
  Real boxhit_tmax = 0;

  const bool hit = BoxRayIntersect(volume_bounds,
      ray_object_space.orig,
      ray_object_space.dir,
      ray_object_space.tmin,
      ray_object_space.tmax,
      &boxhit_tmin, &boxhit_tmax);

  if (!hit) {
    return false;
  }

  interval->tmin = boxhit_tmin;
  interval->tmax = boxhit_tmax;
  interval->object = this;

  return true;
}

bool ObjectInstance::GetVolumeSample(const Vector &point, Real time,
    VolumeSample *sample) const
{
  if (!IsVolume()) {
    return false;
  }

  Transform transform_interp;
  XfmLerpTransformSample(&transform_samples_, time, &transform_interp);

  Vector point_in_objspace = point;
  XfmTransformPointInverse(&transform_interp, &point_in_objspace);

  const bool hit = volume_->GetSample(point_in_objspace, sample);
  return hit;
}

void ObjectInstance::update_bounds()
{
  if (IsSurface()) {
    bounds_ = acc_->GetBounds();
  }
  else if (IsVolume()) {
    bounds_ = volume_->GetBounds();
  }
  else {
    bounds_ = Box();
  }
  merge_sampled_bounds();
}

void ObjectInstance::merge_sampled_bounds()
{
  // assumes bounds_ is the same as acc->bounds or vol->bounds
  Box original_bounds = GetBounds();

  // extend bounds when rotated to ensure object is always inside bounds
  if (transform_samples_.rotate.sample_count > 1) {
    const Real diagonal = BoxDiagonal(original_bounds);
    const Vector centroid = BoxCentroid(original_bounds);

    original_bounds = Box(
        centroid.x - diagonal,
        centroid.y - diagonal,
        centroid.z - diagonal,
        centroid.x + diagonal,
        centroid.y + diagonal,
        centroid.z + diagonal);
  }

  // compute maximum scale over sampling time
  Real S[3] = {0, 0, 0};
  for (int i = 0; i < transform_samples_.scale.sample_count; i++) {
    S[0] = Max(S[0], Abs(transform_samples_.scale.samples[i].vector[0]));
    S[1] = Max(S[1], Abs(transform_samples_.scale.samples[i].vector[1]));
    S[2] = Max(S[2], Abs(transform_samples_.scale.samples[i].vector[2]));
  }

  // accumulate all bounds over sampling time
  Box merged_bounds;
  BoxReverseInfinite(&merged_bounds);

  for (int i = 0; i < transform_samples_.translate.sample_count; i++) {
    const Real *T = transform_samples_.translate.samples[i].vector;
    const Real *R = transform_samples_.rotate.samples[i].vector;
    Transform transform;

    XfmSetTransform(&transform,
      transform_samples_.transform_order, transform_samples_.rotate_order,
      T[0], T[1], T[2],
      R[0], R[1], R[2],
      S[0], S[1], S[2]);

    Box sample_bounds = original_bounds;
    XfmTransformBounds(&transform, &sample_bounds);
    BoxAddBox(&merged_bounds, sample_bounds);
  }

  bounds_ = merged_bounds;
}

} // namespace xxx
