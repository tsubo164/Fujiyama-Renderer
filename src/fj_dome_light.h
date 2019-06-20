// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_DOME_LIGHT_H
#define FJ_DOME_LIGHT_H

#include "fj_light.h"
#include "fj_importance_sampling.h"
#include <vector>

namespace fj {

class DomeLight : public Light {
public:
  DomeLight();
  virtual ~DomeLight();

private:
  virtual int get_sample_count() const;
  virtual void get_samples(LightSample *samples, int max_samples) const;
  virtual Color illuminate(const LightSample &sample, const Vector &Ps) const;
  virtual int preprocess();

  // TODO tmp solution for dome light data
  std::vector<DomeSample> dome_samples_;
};

} // namespace xxx

#endif // FJ_XXX_H
