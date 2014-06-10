/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_object_instance.h"
#include "fj_intersection.h"
#include "fj_accelerator.h"
#include "fj_object_group.h"
#include "fj_interval.h"
#include "fj_property.h"
#include "fj_numeric.h"
#include "fj_memory.h"
#include "fj_vector.h"
#include "fj_volume.h"
#include "fj_matrix.h"
#include "fj_array.h"
#include "fj_ray.h"

#include <assert.h>
#include <stdio.h>
#include <float.h>

namespace fj {

static void update_object_bounds(struct ObjectInstance *obj);
static void merge_sampled_bounds(struct ObjectInstance *obj);

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

void ObjectInstance::SetTranslate(double tx, double ty, double tz, double time)
{
  XfmPushTranslateSample(&transform_samples_, tx, ty, tz, time);
  update_bounds();
}

void ObjectInstance::SetRotate(double rx, double ry, double rz, double time)
{
  XfmPushRotateSample(&transform_samples_, rx, ry, rz, time);
  update_bounds();
}

void ObjectInstance::SetScale(double sx, double sy, double sz, double time)
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

bool ObjectInstance::Intersect(const Ray &ray, double time, Intersection *isect) const
{
  Transform transform_interp;
  Ray ray_object_space;
  int hit = 0;

  if (!IsSurface())
    return 0;

  XfmLerpTransformSample(&transform_samples_, time, &transform_interp);

  // transform ray to object space
  ray_object_space = ray;
  XfmTransformPointInverse(&transform_interp, &ray_object_space.orig);
  XfmTransformVectorInverse(&transform_interp, &ray_object_space.dir);

  hit = acc_->Intersect(ray_object_space, time, isect);
  if (!hit)
    return 0;

  // transform intersection back to world space
  XfmTransformPoint(&transform_interp, &isect->P);
  XfmTransformVector(&transform_interp, &isect->N);
  Normalize(&isect->N);

  XfmTransformVector(&transform_interp, &isect->dPdu);
  XfmTransformVector(&transform_interp, &isect->dPdv);

  isect->object = this;

  return 1;
}

bool ObjectInstance::VolumeIntersect(const Ray &ray, double time, Interval *interval) const
{
  struct Transform transform_interp;
  struct Ray ray_object_space;
  struct Box volume_bounds;
  double boxhit_tmin = 0;
  double boxhit_tmax = 0;
  int hit = 0;

  if (!IsVolume())
    return 0;

  volume_bounds = volume_->GetBounds();

  XfmLerpTransformSample(&transform_samples_, time, &transform_interp);

  /* transform ray to object space */
  ray_object_space = ray;
  XfmTransformPointInverse(&transform_interp, &ray_object_space.orig);
  XfmTransformVectorInverse(&transform_interp, &ray_object_space.dir);

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
  interval->object = this;

  return 1;
}

bool ObjectInstance::GetVolumeSample(const Vector &point, double time,
    VolumeSample *sample) const
{
  struct Transform transform_interp;
  struct Vector point_in_objspace;
  int hit = 0;

  if (!IsVolume())
    return 0;

  XfmLerpTransformSample(&transform_samples_, time, &transform_interp);

  point_in_objspace = point;
  XfmTransformPointInverse(&transform_interp, &point_in_objspace);

  hit = volume_->GetSample(point_in_objspace, sample);

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
  Box merged_bounds(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
  Box original_bounds(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
  double S[3] = {1, 1, 1};
  int i;

  // assumes bounds_ is the same as acc->bounds or vol->bounds
  original_bounds = GetBounds();

  // extend bounds when rotated to ensure object is always inside bounds
  if (transform_samples_.rotate.sample_count > 1) {
    const double diagonal = BoxDiagonal(original_bounds);
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
  S[0] = transform_samples_.scale.samples[0].vector[0];
  S[1] = transform_samples_.scale.samples[0].vector[1];
  S[2] = transform_samples_.scale.samples[0].vector[2];
  for (i = 1; i < transform_samples_.scale.sample_count; i++) {
    S[0] = Max(S[0], transform_samples_.scale.samples[i].vector[0]);
    S[1] = Max(S[1], transform_samples_.scale.samples[i].vector[1]);
    S[2] = Max(S[2], transform_samples_.scale.samples[i].vector[2]);
  }

  // accumulate all bounds over sampling time
  for (i = 0; i < transform_samples_.translate.sample_count; i++) {
    Box sample_bounds(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
    const double *T = transform_samples_.translate.samples[i].vector;
    const double *R = transform_samples_.rotate.samples[i].vector;
    Transform transform;

    XfmSetTransform(&transform,
      transform_samples_.transform_order, transform_samples_.rotate_order,
      T[0], T[1], T[2],
      R[0], R[1], R[2],
      S[0], S[1], S[2]);

    sample_bounds = original_bounds;
    XfmTransformBounds(&transform, &sample_bounds);
    BoxAddBox(&merged_bounds, sample_bounds);
  }

  bounds_ = merged_bounds;
}

struct ObjectInstance *ObjNew(void)
{
  return new ObjectInstance();
}

void ObjFree(struct ObjectInstance *obj)
{
  delete obj;
}

int ObjSetSurface(struct ObjectInstance *obj, const struct Accelerator *acc)
{
  if (obj->acc_ != NULL)
    return -1;

  if (obj->volume_ != NULL)
    return -1;

  obj->acc_ = acc;
  update_object_bounds(obj);

  assert(obj->acc_ != NULL && obj->volume_ == NULL);
  return 0;
}

int ObjSetVolume(struct ObjectInstance *obj, const struct Volume *volume)
{
  if (obj->acc_ != NULL)
    return -1;

  if (obj->volume_ != NULL)
    return -1;

  obj->volume_ = volume;
  update_object_bounds(obj);

  assert(obj->acc_ == NULL && obj->volume_ != NULL);
  return 0;
}

int ObjIsSurface(const struct ObjectInstance *obj)
{
  if (obj->acc_ == NULL)
    return 0;

  assert(obj->volume_ == NULL);
  return 1;
}

int ObjIsVolume(const struct ObjectInstance *obj)
{
  if (obj->volume_ == NULL)
    return 0;

  assert(obj->acc_ == NULL);
  return 1;
}

void ObjSetTranslate(struct ObjectInstance *obj,
    double tx, double ty, double tz, double time)
{
  XfmPushTranslateSample(&obj->transform_samples_, tx, ty, tz, time);
  update_object_bounds(obj);
}

void ObjSetRotate(struct ObjectInstance *obj,
    double rx, double ry, double rz, double time)
{
  XfmPushRotateSample(&obj->transform_samples_, rx, ry, rz, time);
  update_object_bounds(obj);
}

void ObjSetScale(struct ObjectInstance *obj,
    double sx, double sy, double sz, double time)
{
  XfmPushScaleSample(&obj->transform_samples_, sx, sy, sz, time);
  update_object_bounds(obj);
}

void ObjSetTransformOrder(struct ObjectInstance *obj, int order)
{
  XfmSetSampleTransformOrder(&obj->transform_samples_, order);
  update_object_bounds(obj);
}

void ObjSetRotateOrder(struct ObjectInstance *obj, int order)
{
  XfmSetSampleRotateOrder(&obj->transform_samples_, order);
  update_object_bounds(obj);
}

void ObjSetShader(struct ObjectInstance *obj, const struct Shader *shader)
{
  obj->shader_ = shader;
}

void ObjSetLightList(struct ObjectInstance *obj, const struct Light **lights, int count)
{
  obj->target_lights_ = lights;
  obj->n_target_lights_ = count;
}

void ObjSetReflectTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp)
{
  obj->reflection_target_ = grp;
}

void ObjSetRefractTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp)
{
  obj->refraction_target_ = grp;
}

void ObjSetShadowTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp)
{
  obj->shadow_target_ = grp;
}

/* TODO come up with better way to make self target */
void ObjSetSelfHitTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp)
{
  obj->self_target_ = grp;
}

const struct ObjectGroup *ObjGetReflectTarget(const struct ObjectInstance *obj)
{
  return obj->reflection_target_;
}

const struct ObjectGroup *ObjGetRefractTarget(const struct ObjectInstance *obj)
{
  return obj->refraction_target_;
}

const struct ObjectGroup *ObjGetShadowTarget(const struct ObjectInstance *obj)
{
  return obj->shadow_target_;
}

const struct ObjectGroup *ObjGetSelfHitTarget(const struct ObjectInstance *obj)
{
  return obj->self_target_;
}

const struct Shader *ObjGetShader(const struct ObjectInstance *obj)
{
  return obj->shader_;
}

const struct Light **ObjGetLightList(const struct ObjectInstance *obj)
{
  return obj->target_lights_;
}

int ObjGetLightCount(const struct ObjectInstance *obj)
{
  return obj->n_target_lights_;
}

void ObjGetBounds(const struct ObjectInstance *obj, struct Box *bounds)
{
  *bounds = obj->bounds_;
}

void ObjComputeBounds(struct ObjectInstance *obj)
{
  update_object_bounds(obj);
}

int ObjIntersect(const struct ObjectInstance *obj, double time,
    const struct Ray *ray, struct Intersection *isect)
{
  struct Transform transform_interp;
  struct Ray ray_object_space;
  int hit = 0;

  if (!ObjIsSurface(obj))
    return 0;

  XfmLerpTransformSample(&obj->transform_samples_, time, &transform_interp);

  /* transform ray to object space */
  ray_object_space = *ray;
  XfmTransformPointInverse(&transform_interp, &ray_object_space.orig);
  XfmTransformVectorInverse(&transform_interp, &ray_object_space.dir);

  hit = obj->acc_->Intersect(ray_object_space, time, isect);
  if (!hit)
    return 0;

  /* transform intersection back to world space */
  XfmTransformPoint(&transform_interp, &isect->P);
  XfmTransformVector(&transform_interp, &isect->N);
  Normalize(&isect->N);

  XfmTransformVector(&transform_interp, &isect->dPdu);
  XfmTransformVector(&transform_interp, &isect->dPdv);

  isect->object = obj;

  return 1;
}

int ObjVolumeIntersect(const struct ObjectInstance *obj, double time,
      const struct Ray *ray, struct Interval *interval)
{
  struct Transform transform_interp;
  struct Ray ray_object_space;
  struct Box volume_bounds;
  double boxhit_tmin = 0;
  double boxhit_tmax = 0;
  int hit = 0;

  if (!ObjIsVolume(obj))
    return 0;

  volume_bounds = obj->volume_->GetBounds();

  XfmLerpTransformSample(&obj->transform_samples_, time, &transform_interp);

  /* transform ray to object space */
  ray_object_space = *ray;
  XfmTransformPointInverse(&transform_interp, &ray_object_space.orig);
  XfmTransformVectorInverse(&transform_interp, &ray_object_space.dir);

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

int ObjGetVolumeSample(const struct ObjectInstance *obj, double time,
      const struct Vector *point, struct VolumeSample *sample)
{
  struct Transform transform_interp;
  struct Vector point_in_objspace;
  int hit = 0;

  if (!ObjIsVolume(obj))
    return 0;

  XfmLerpTransformSample(&obj->transform_samples_, time, &transform_interp);

  point_in_objspace = *point;
  XfmTransformPointInverse(&transform_interp, &point_in_objspace);

  hit = obj->volume_->GetSample(point_in_objspace, sample);

  return hit;
}

static void update_object_bounds(struct ObjectInstance *obj)
{
  if (ObjIsSurface(obj)) {
    obj->bounds_ = obj->acc_->GetBounds();
  }
  else if (ObjIsVolume(obj)) {
    obj->bounds_ = obj->volume_->GetBounds();
  }
  else {
    obj->bounds_ = Box();
  }
  merge_sampled_bounds(obj);
}

static void merge_sampled_bounds(struct ObjectInstance *obj)
{
  struct Box merged_bounds(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
  struct Box original_bounds(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
  double S[3] = {1, 1, 1};
  int i;

  /* assumes obj->bounds_ is the same as acc->bounds or vol->bounds */
  ObjGetBounds(obj, &original_bounds);

  /* extend bounds when rotated to ensure object is always inside bounds */
  if (obj->transform_samples_.rotate.sample_count > 1) {
    const double diagonal = BoxDiagonal(original_bounds);
    const Vector centroid = BoxCentroid(original_bounds);

    original_bounds = Box(
        centroid.x - diagonal,
        centroid.y - diagonal,
        centroid.z - diagonal,
        centroid.x + diagonal,
        centroid.y + diagonal,
        centroid.z + diagonal);
  }

  /* compute maximum scale over sampling time */
  S[0] = obj->transform_samples_.scale.samples[0].vector[0];
  S[1] = obj->transform_samples_.scale.samples[0].vector[1];
  S[2] = obj->transform_samples_.scale.samples[0].vector[2];
  for (i = 1; i < obj->transform_samples_.scale.sample_count; i++) {
    S[0] = Max(S[0], obj->transform_samples_.scale.samples[i].vector[0]);
    S[1] = Max(S[1], obj->transform_samples_.scale.samples[i].vector[1]);
    S[2] = Max(S[2], obj->transform_samples_.scale.samples[i].vector[2]);
  }

  /* accumulate all bounds over sampling time */
  for (i = 0; i < obj->transform_samples_.translate.sample_count; i++) {
    struct Box sample_bounds(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
    const double *T = obj->transform_samples_.translate.samples[i].vector;
    const double *R = obj->transform_samples_.rotate.samples[i].vector;
    struct Transform transform;

    XfmSetTransform(&transform,
      obj->transform_samples_.transform_order, obj->transform_samples_.rotate_order,
      T[0], T[1], T[2],
      R[0], R[1], R[2],
      S[0], S[1], S[2]);

    sample_bounds = original_bounds;
    XfmTransformBounds(&transform, &sample_bounds);
    BoxAddBox(&merged_bounds, sample_bounds);
  }

  obj->bounds_ = merged_bounds;
}

} // namespace xxx
