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
#include "fj_box.h"
#include "fj_ray.h"

#include <assert.h>
#include <stdio.h>
#include <float.h>

namespace fj {

struct ObjectInstance {
  /* geometric properties */
  const struct Accelerator *acc;
  const struct Volume *volume;
  struct Box bounds;

  /* transformation properties */
  struct TransformSampleList transform_samples;

  /* non-geometric properties */
  const struct Shader *shader;
  const struct Light **target_lights;
  int n_target_lights;
  const struct ObjectGroup *reflection_target;
  const struct ObjectGroup *refraction_target;
  const struct ObjectGroup *shadow_target;
  const struct ObjectGroup *self_target;
};

static void update_object_bounds(struct ObjectInstance *obj);
static void merge_sampled_bounds(struct ObjectInstance *obj);

/* ObjectInstance interfaces */
struct ObjectInstance *ObjNew(void)
{
  struct ObjectInstance *obj = FJ_MEM_ALLOC(struct ObjectInstance);

  if (obj == NULL)
    return NULL;

  obj->acc = NULL;
  obj->volume = NULL;

  XfmInitTransformSampleList(&obj->transform_samples);
  update_object_bounds(obj);

  obj->shader = NULL;
  obj->target_lights = NULL;
  obj->n_target_lights = 0;

  obj->reflection_target = NULL;
  obj->refraction_target = NULL;
  obj->shadow_target = NULL;
  obj->self_target = NULL;

  return obj;
}

void ObjFree(struct ObjectInstance *obj)
{
  if (obj == NULL)
    return;
  FJ_MEM_FREE(obj);
}

int ObjSetSurface(struct ObjectInstance *obj, const struct Accelerator *acc)
{
  if (obj->acc != NULL)
    return -1;

  if (obj->volume != NULL)
    return -1;

  obj->acc = acc;
  update_object_bounds(obj);

  assert(obj->acc != NULL && obj->volume == NULL);
  return 0;
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

void ObjSetTranslate(struct ObjectInstance *obj,
    double tx, double ty, double tz, double time)
{
  XfmPushTranslateSample(&obj->transform_samples, tx, ty, tz, time);
  update_object_bounds(obj);
}

void ObjSetRotate(struct ObjectInstance *obj,
    double rx, double ry, double rz, double time)
{
  XfmPushRotateSample(&obj->transform_samples, rx, ry, rz, time);
  update_object_bounds(obj);
}

void ObjSetScale(struct ObjectInstance *obj,
    double sx, double sy, double sz, double time)
{
  XfmPushScaleSample(&obj->transform_samples, sx, sy, sz, time);
  update_object_bounds(obj);
}

void ObjSetTransformOrder(struct ObjectInstance *obj, int order)
{
  XfmSetSampleTransformOrder(&obj->transform_samples, order);
  update_object_bounds(obj);
}

void ObjSetRotateOrder(struct ObjectInstance *obj, int order)
{
  XfmSetSampleRotateOrder(&obj->transform_samples, order);
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
  obj->reflection_target = grp;
}

void ObjSetRefractTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp)
{
  obj->refraction_target = grp;
}

void ObjSetShadowTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp)
{
  obj->shadow_target = grp;
}

/* TODO come up with better way to make self target */
void ObjSetSelfHitTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp)
{
  obj->self_target = grp;
}

const struct ObjectGroup *ObjGetReflectTarget(const struct ObjectInstance *obj)
{
  return obj->reflection_target;
}

const struct ObjectGroup *ObjGetRefractTarget(const struct ObjectInstance *obj)
{
  return obj->refraction_target;
}

const struct ObjectGroup *ObjGetShadowTarget(const struct ObjectInstance *obj)
{
  return obj->shadow_target;
}

const struct ObjectGroup *ObjGetSelfHitTarget(const struct ObjectInstance *obj)
{
  return obj->self_target;
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

void ObjGetBounds(const struct ObjectInstance *obj, struct Box *bounds)
{
  *bounds = obj->bounds;
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

  XfmLerpTransformSample(&obj->transform_samples, time, &transform_interp);

  /* transform ray to object space */
  ray_object_space = *ray;
  XfmTransformPointInverse(&transform_interp, &ray_object_space.orig);
  XfmTransformVectorInverse(&transform_interp, &ray_object_space.dir);

  hit = AccIntersect(obj->acc, time, &ray_object_space, isect);
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

  VolGetBounds(obj->volume, &volume_bounds);

  XfmLerpTransformSample(&obj->transform_samples, time, &transform_interp);

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

  XfmLerpTransformSample(&obj->transform_samples, time, &transform_interp);

  point_in_objspace = *point;
  XfmTransformPointInverse(&transform_interp, &point_in_objspace);

  hit = VolGetSample(obj->volume, &point_in_objspace, sample);

  return hit;
}

static void update_object_bounds(struct ObjectInstance *obj)
{
  if (ObjIsSurface(obj)) {
    AccGetBounds(obj->acc, &obj->bounds);
  }
  else if (ObjIsVolume(obj)) {
    VolGetBounds(obj->volume, &obj->bounds);
  }
  else {
    obj->bounds = Box();
  }
  merge_sampled_bounds(obj);
}

static void merge_sampled_bounds(struct ObjectInstance *obj)
{
  struct Box merged_bounds(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
  struct Box original_bounds(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
  double S[3] = {1, 1, 1};
  int i;

  /* assumes obj->bounds is the same as acc->bounds or vol->bounds */
  ObjGetBounds(obj, &original_bounds);

  /* extend bounds when rotated to ensure object is always inside bounds */
  if (obj->transform_samples.rotate.sample_count > 1) {
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
  S[0] = obj->transform_samples.scale.samples[0].vector[0];
  S[1] = obj->transform_samples.scale.samples[0].vector[1];
  S[2] = obj->transform_samples.scale.samples[0].vector[2];
  for (i = 1; i < obj->transform_samples.scale.sample_count; i++) {
    S[0] = Max(S[0], obj->transform_samples.scale.samples[i].vector[0]);
    S[1] = Max(S[1], obj->transform_samples.scale.samples[i].vector[1]);
    S[2] = Max(S[2], obj->transform_samples.scale.samples[i].vector[2]);
  }

  /* accumulate all bounds over sampling time */
  for (i = 0; i < obj->transform_samples.translate.sample_count; i++) {
    struct Box sample_bounds(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
    const double *T = obj->transform_samples.translate.samples[i].vector;
    const double *R = obj->transform_samples.rotate.samples[i].vector;
    struct Transform transform;

    XfmSetTransform(&transform,
      obj->transform_samples.transform_order, obj->transform_samples.rotate_order,
      T[0], T[1], T[2],
      R[0], R[1], R[2],
      S[0], S[1], S[2]);

    sample_bounds = original_bounds;
    XfmTransformBounds(&transform, &sample_bounds);
    BoxAddBox(&merged_bounds, sample_bounds);
  }

  obj->bounds = merged_bounds;
}

} // namespace xxx
