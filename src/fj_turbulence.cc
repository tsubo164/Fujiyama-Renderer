// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_turbulence.h"
#include "fj_noise.h"
#include <cassert>

namespace fj {

Turbulence::Turbulence() :
    amplitude_  (1, 1, 1),
    frequency_  (1, 1, 1),
    offset_     (0, 0, 0),
    lacunarity_ (2),
    gain_       (.5),
    octaves_    (8)
{
}

Turbulence::~Turbulence()
{
}

void Turbulence::SetAmplitude(Real x, Real y, Real z)
{
  amplitude_ = Vector(x, y, z);
}

void Turbulence::SetFrequency(Real x, Real y, Real z)
{
  frequency_ = Vector(x, y, z);
}

void Turbulence::SetOffset(Real x, Real y, Real z)
{
  offset_ = Vector(x, y, z);
}

void Turbulence::SetLacunarity(Real lacunarity)
{
  lacunarity_ = lacunarity;
}

void Turbulence::SetGain(Real gain)
{
  gain_ = gain;
}

void Turbulence::SetOctaves(int octaves)
{
  assert(octaves > 0);
  octaves_ = octaves;
}

Real Turbulence::Evaluate(const Vector &position) const
{
  const Vector P = position * frequency_ + offset_;
  const Real noise = PerlinNoise(P, lacunarity_, gain_, octaves_);

  return amplitude_.x * noise;
}

Vector Turbulence::Evaluate3d(const Vector &position) const
{
  const Vector P = position * frequency_ + offset_;
  const Vector noise = PerlinNoise3d(P, lacunarity_, gain_, octaves_);

  return amplitude_ * noise;
}

} // namespace xxx
