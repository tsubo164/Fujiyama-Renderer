/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef NOISE_H
#define NOISE_H

#ifdef __cplusplus
extern "C" {
#endif

struct Vector;

extern double PerlinNoise(const struct Vector *position,
    double lacunarity, double persistence, int octaves);

extern void PerlinNoise3d(const struct Vector *position,
    double lacunarity, double persistence, int octaves,
    struct Vector *P_out);

extern double PeriodicNoise3d(double x, double y, double z);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

