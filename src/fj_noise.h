/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_NOISE_H
#define FJ_NOISE_H

namespace  fj {

struct Vector;

extern double PerlinNoise(const struct Vector *position,
    double lacunarity, double persistence, int octaves);

extern void PerlinNoise3d(const struct Vector *position,
    double lacunarity, double persistence, int octaves,
    struct Vector *P_out);

extern double PeriodicNoise3d(double x, double y, double z);

} // namespace xxx

#endif /* FJ_XXX_H */
