/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef LOCALGEOMETRY_H
#define LOCALGEOMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

struct ObjectInstance;

struct LocalGeometry {
	double P[3];
	double N[3];
	float Cd[3];
	float uv[2];

	/* TODO TEST */
	double T[3];

	const struct ObjectInstance *object;
	const void *geometry;
	int prim_id;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

