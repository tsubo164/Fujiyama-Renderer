// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_TURBULENCE_H
#define FJ_TURBULENCE_H

#include "fj_compatibility.h"
#include "fj_vector.h"
#include "fj_types.h"

namespace fj {

class FJ_API Turbulence {
public:
  Turbulence();
  ~Turbulence();

  void SetAmplitude(Real x, Real y, Real z);
  void SetFrequency(Real x, Real y, Real z);
  void SetOffset(Real x, Real y, Real z);
  void SetLacunarity(Real lacunarity);
  void SetGain(Real gain);
  void SetOctaves(int octaves);

  double Evaluate(const Vector &position) const;
  Vector Evaluate3d(const Vector &position) const;

private:
  Vector amplitude_;
  Vector frequency_;
  Vector offset_;
  Real   lacunarity_;
  Real   gain_;
  int    octaves_;
};

} // namespace xxx

#endif // FJ_XXX_H
