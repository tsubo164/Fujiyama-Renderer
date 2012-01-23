/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef CURVE_H
#define CURVE_H

#ifdef __cplusplus
extern "C" {
#endif

struct Curve;
struct Accelerator;

/* TODO temporary */
struct Curve {
	double bounds[6];

	double *P;
	double *width;
	float *Cd;
	float *uv;
	int *indices;

	int nverts;
	int ncurves;
};

extern struct Curve *CrvNew(void);
extern void CrvFree(struct Curve *light);

extern void *CrvAllocateVertex(struct Curve *curve, const char *attr_name, int nverts);
extern void *CrvAllocateCurve(struct Curve *curve, const char *attr_name, int ncurves);

extern void CrvComputeBounds(struct Curve *curve);

extern void CrvSetupAccelerator(const struct Curve *curve, struct Accelerator *acc);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

