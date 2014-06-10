/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_OBJECTINSTANCE_H
#define FJ_OBJECTINSTANCE_H

#include "fj_transform.h"
#include "fj_box.h"

namespace fj {

struct Intersection;
struct VolumeSample;
struct Accelerator;
struct Interval;
struct Shader;
struct Volume;
struct Light;
struct Box;
struct Ray;

class ObjectGroup;

class ObjectInstance {
public:
  ObjectInstance();
  ~ObjectInstance();

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
  bool Intersect(const Ray &ray, double time, Intersection *isect) const;
  bool VolumeIntersect(const Ray &ray, double time, Interval *interval) const;
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

/* ObjectInstance interfaces */
extern struct ObjectInstance *ObjNew(void);
extern void ObjFree(struct ObjectInstance *obj);

extern int ObjSetSurface(struct ObjectInstance *obj, const struct Accelerator *acc);
extern int ObjSetVolume(struct ObjectInstance *obj, const struct Volume *volume);
extern int ObjIsSurface(const struct ObjectInstance *obj);
extern int ObjIsVolume(const struct ObjectInstance *obj);

/* transformation */
extern void ObjSetTranslate(struct ObjectInstance *obj,
    double tx, double ty, double tz, double time);
extern void ObjSetRotate(struct ObjectInstance *obj,
    double rx, double ry, double rz, double time);
extern void ObjSetScale(struct ObjectInstance *obj,
    double sx, double sy, double sz, double time);
extern void ObjSetTransformOrder(struct ObjectInstance *obj, int order);
extern void ObjSetRotateOrder(struct ObjectInstance *obj, int order);

/* non-geometric properties */
extern void ObjSetShader(struct ObjectInstance *obj, const struct Shader *shader);
extern void ObjSetLightList(struct ObjectInstance *obj, const struct Light **lights, int count);
extern void ObjSetReflectTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp);
extern void ObjSetRefractTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp);
extern void ObjSetShadowTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp);
extern void ObjSetSelfHitTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp);

extern const struct ObjectGroup *ObjGetReflectTarget(const struct ObjectInstance *obj);
extern const struct ObjectGroup *ObjGetRefractTarget(const struct ObjectInstance *obj);
extern const struct ObjectGroup *ObjGetShadowTarget(const struct ObjectInstance *obj);
extern const struct ObjectGroup *ObjGetSelfHitTarget(const struct ObjectInstance *obj);

extern const struct Shader *ObjGetShader(const struct ObjectInstance *obj);
extern const struct Light **ObjGetLightList(const struct ObjectInstance *obj);
extern int ObjGetLightCount(const struct ObjectInstance *obj);
extern void ObjGetBounds(const struct ObjectInstance *obj, struct Box *bounds);
extern void ObjComputeBounds(struct ObjectInstance *obj);

/* sampling */
extern int ObjIntersect(const struct ObjectInstance *obj, double time,
      const struct Ray *ray, struct Intersection *isect);
extern int ObjVolumeIntersect(const struct ObjectInstance *obj, double time,
      const struct Ray *ray, struct Interval *interval);
extern int ObjGetVolumeSample(const struct ObjectInstance *obj, double time,
      const struct Vector *point, struct VolumeSample *sample);

} // namespace xxx

#endif // FJ_XXX_H
