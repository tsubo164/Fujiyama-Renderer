/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TRIANGLE_H
#define TRIANGLE_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
	DO_NOT_CULL_BACKFACES = 0,
	CULL_BACKFACES = 1
};

extern double TriComputeArea(const double *vert0, const double *vert1, const double *vert2);
extern void TriComputeBounds(
		double *box,
		const double *vert0, const double *vert1, const double *vert2);
extern void TriComputeFaceNormal(
		double *N,
		const double *vert0, const double *vert1, const double *vert2);
extern void TriComputeNormal(
		double *N,
		const double *N0, const double *N1, const double *N2,
		double u, double v);

extern int TriRayIntersect(
		const double *vert0, const double *vert1, const double *vert2,
		const double *orig, const double *dir, int cull_backfaces,
		double *t, double *u, double *v);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

