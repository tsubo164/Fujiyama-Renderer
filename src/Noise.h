/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef NOISE_H
#define NOISE_H

#ifdef __cplusplus
extern "C" {
#endif

extern void PerlinNoise(const double *position, const double *amplitude,
		const double *frequency, const double *offset,
		double lacunarity, double persistence, int octaves, double *result);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

