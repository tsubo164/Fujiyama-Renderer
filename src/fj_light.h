// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_LIGHT_H
#define FJ_LIGHT_H

#include "fj_compatibility.h"
#include "fj_transform.h"
#include "fj_random.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_types.h"
#include <vector>

namespace fj {

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
  virtual ~Light();

  // light properties
  void SetColor(float r, float g, float b);
  void SetIntensity(float intensity);
  //TODO this needs to be void SetSampleDensity(float density)
  void SetSampleCount(int sample_count);
  void SetDoubleSided(bool on_or_off);
  void SetEnvironmentMap(Texture *texture);

  Color GetColor() const;
  float GetIntensity() const;
  int GetSampleDensity() const;
  bool IsDoulbeSided() const;
  Texture *GetEnvironmentMap() const;

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

protected:
  void get_transform_sample(Transform &sample, Real time) const;
  //TODO this may not be necessary
  float get_sample_intensity() const;

private:
  Color color_;
  float intensity_;

  TransformSampleList transform_samples_;
  bool double_sided_;
  //TODO this needs to be sample_density_
  int sample_count_;
  float sample_intensity_;
  Texture *environment_map_;

private:
  virtual int get_sample_count() const = 0;
  virtual void get_samples(LightSample *samples, int max_samples) const = 0;
  virtual Color illuminate(const LightSample &sample, const Vector &Ps) const = 0;
  virtual int preprocess() = 0;
};

} // namespace xxx

#endif // FJ_XXX_H
