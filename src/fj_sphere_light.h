// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SPHERE_LIGHT_H
#define FJ_SPHERE_LIGHT_H

#include "fj_light.h"
#include "fj_random.h"

namespace fj {

class SphereLight : public Light {
public:
  SphereLight();
  virtual ~SphereLight();

private:
  virtual int get_sample_count() const;
  virtual void get_samples(LightSample *samples, int max_samples) const;
  virtual Color illuminate(const LightSample &sample, const Vector &Ps) const;
  virtual int preprocess();

  XorShift rng_;
};

} // namespace xxx

#endif // FJ_XXX_H
