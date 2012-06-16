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
		const double *frequency, const double *offset,
		double lacunarity, double persistence, int octaves);

extern void PerlinNoise3d(const double *position,
		const double *frequency, const double *offset,
		double lacunarity, double persistence, int octaves,
		double *P_out);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

