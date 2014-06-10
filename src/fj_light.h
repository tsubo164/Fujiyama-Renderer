/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_LIGHT_H
#define FJ_LIGHT_H

#include "fj_importance_sampling.h"
#include "fj_transform.h"
#include "fj_random.h"
#include "fj_vector.h"
#include "fj_color.h"

namespace fj {

enum LightType {
  LGT_POINT = 0,
  LGT_GRID,
  LGT_SPHERE,
  LGT_DOME
};

class Light;
class Texture;

class LightSample {
public:
  const Light *light;
  Vector P;
  Vector N;
  Color color;
};

class Light {
public:
  Light();
  ~Light();

  void SetLightType(int light_type);

  // light properties
  void SetColor(float r, float g, float b);
  void SetIntensity(float intensity);
  void SetSampleCount(int sample_count);
  void SetDoubleSided(bool on_or_off);
  void SetEnvironmentMap(Texture *texture);

  // transformation
  void SetTranslate(double tx, double ty, double tz, double time);
  void SetRotate(double rx, double ry, double rz, double time);
  void SetScale(double sx, double sy, double sz, double time);
  void SetTransformOrder(int order);
  void SetRotateOrder(int order);

  // samples
  void GetSamples(LightSample *samples, int max_samples) const;
  int GetSampleCount() const;
  Color Illuminate(const LightSample &sample, const Vector &Ps) const;
  int Preprocess();

public:
  Color color_;
  float intensity_;

  // transformation properties
  TransformSampleList transform_samples_;

  // rng
  XorShift xr_;

  int type_;
  bool double_sided_;
  int sample_count_;
  float sample_intensity_;

  Texture *environment_map_;
  // TODO tmp solution for dome light data
  DomeSample *dome_samples_;

  // TODO USE INHERITANCE
  // functions
  int (*GetSampleCount_)(const Light *light);
  void (*GetSamples_)(const Light *light,
      LightSample *samples, int max_samples);
  void (*Illuminate_)(const Light *light,
      const LightSample *sample,
      const Vector *Ps, Color *Cl);
  int (*Preprocess_)(Light *light);
};

extern Light *LgtNew(int light_type);
extern void LgtFree(Light *light);

extern void LgtSetColor(struct Light *light, float r, float g, float b);
extern void LgtSetIntensity(struct Light *light, double intensity);
extern void LgtSetSampleCount(struct Light *light, int sample_count);
extern void LgtSetDoubleSided(struct Light *light, int on_or_off);

extern void LgtSetEnvironmentMap(struct Light *light, struct Texture *texture);

/* transformation */
extern void LgtSetTranslate(struct Light *light,
    double tx, double ty, double tz, double time);
extern void LgtSetRotate(struct Light *light,
    double rx, double ry, double rz, double time);
extern void LgtSetScale(struct Light *light,
    double sx, double sy, double sz, double time);
extern void LgtSetTransformOrder(struct Light *light, int order);
extern void LgtSetRotateOrder(struct Light *light, int order);

/* samples */
extern void LgtGetSamples(const struct Light *light,
    struct LightSample *samples, int max_samples);
extern int LgtGetSampleCount(const struct Light *light);
extern void LgtIlluminate(const struct LightSample *sample,
    const struct Vector *Ps, struct Color *Cl);

extern int LgtPreprocess(struct Light *light);

} // namespace fj

#endif // FJ_XXX_H
