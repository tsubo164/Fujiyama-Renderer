// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_LIGHT_H
#define FJ_LIGHT_H

#include "fj_importance_sampling.h"
#include "fj_transform.h"
#include "fj_random.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_types.h"
#include <vector>

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
  LightSample() : light(NULL), P(), N(), color() {}
  ~LightSample() {}

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
  void SetTranslate(Real tx, Real ty, Real tz, Real time);
  void SetRotate(Real rx, Real ry, Real rz, Real time);
  void SetScale(Real sx, Real sy, Real sz, Real time);
  void SetTransformOrder(int order);
  void SetRotateOrder(int order);

  // samples
  void GetSamples(LightSample *samples, int max_samples) const;
  int GetSampleCount() const;
  Color Illuminate(const LightSample &sample, const Vector &Ps) const;
  int Preprocess();

public: // TODO ONCE FINISHING INHERITANCE MAKE IT PRAIVATE
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
  std::vector<DomeSample> dome_samples_;

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

} // namespace xxx

#endif // FJ_XXX_H
