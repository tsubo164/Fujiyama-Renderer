// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_POINT_LIGHT_H
#define FJ_POINT_LIGHT_H

#include "fj_light.h"

namespace fj {

class PointLight : public Light {
public:
  PointLight();
  virtual ~PointLight();

private:
  virtual int get_sample_count() const;
  virtual void get_samples(LightSample *samples, int max_samples) const;
  virtual Color illuminate(const LightSample &sample, const Vector &Ps) const;
  virtual int preprocess();
};

} // namespace xxx

#endif // FJ_XXX_H
