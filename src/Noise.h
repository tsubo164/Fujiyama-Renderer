/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef NOISE_H
#define NOISE_H

#ifdef __cplusplus
extern "C" {
#endif

extern double PerlinNoise(const double *position,
		double lacunarity, double persistence, int octaves);

extern void PerlinNoise3d(const double *position,
		double lacunarity, double persistence, int octaves,
		double *P_out);

/* periodic noise 3d */
extern double PNoise3d(double x, double y, double z);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

