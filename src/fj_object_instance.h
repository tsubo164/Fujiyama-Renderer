/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_OBJECTINSTANCE_H
#define FJ_OBJECTINSTANCE_H

#include "fj_transform.h"
#include "fj_box.h"

namespace fj {

class Intersection;
class VolumeSample;
class Accelerator;
class Interval;
class Shader;
class Volume;
class Light;
class Ray;

class ObjectGroup;

class ObjectInstance {
public:
  ObjectInstance();
  ~ObjectInstance();

  // surface/volume interfaces
  int SetSurface(const Accelerator *acc);
  int SetVolume(const Volume *volume);
  bool IsSurface() const;
  bool IsVolume() const;

  // transformation
  void SetTranslate(double tx, double ty, double tz, double time);
  void SetRotate(double rx, double ry, double rz, double time);
  void SetScale(double sx, double sy, double sz, double time);
  void SetTransformOrder(int order);
  void SetRotateOrder(int order);

  // non-geometric properties */
  void SetShader(const Shader *shader);
  void SetLightList(const Light **lights, int count);
  void SetReflectTarget(const ObjectGroup *group);
  void SetRefractTarget(const ObjectGroup *group);
  void SetShadowTarget(const ObjectGroup *group);
  void SetSelfHitTarget(const ObjectGroup *group);

  const ObjectGroup *GetReflectTarget() const;
  const ObjectGroup *GetRefractTarget() const;
  const ObjectGroup *GetShadowTarget() const;
  const ObjectGroup *GetSelfHitTarget() const;

  const Shader *GetShader() const;
  const Light **GetLightList() const;
  int GetLightCount() const;
  const Box &GetBounds() const;
  void ComputeBounds();

  // sampling
  bool RayIntersect(const Ray &ray, double time, Intersection *isect) const;
  bool RayVolumeIntersect(const Ray &ray, double time, Interval *interval) const;
  bool GetVolumeSample(const Vector &point, double time, VolumeSample *sample) const;

public:
  void update_bounds();
  void merge_sampled_bounds();

  // geometric properties
  const Accelerator *acc_;
  const Volume *volume_;
  Box bounds_;

  // transformation properties
  TransformSampleList transform_samples_;

  // non-geometric properties
  const Shader *shader_;
  const Light **target_lights_;
  int n_target_lights_;
  const ObjectGroup *reflection_target_;
  const ObjectGroup *refraction_target_;
  const ObjectGroup *shadow_target_;
  const ObjectGroup *self_target_;
};

extern ObjectInstance *ObjNew(void);
extern void ObjFree(ObjectInstance *obj);

} // namespace xxx

#endif // FJ_XXX_H
