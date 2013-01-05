/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef RANDOM_H
#define RANDOM_H

#ifdef __cplusplus
extern "C" {
#endif

struct XorShift {
	unsigned long state[4];
};

extern void XorInit(struct XorShift *xr);

extern unsigned long XorNextInteger(struct XorShift *xr);
extern double XorNextFloat01(struct XorShift *xr);
extern void XorSolidSphereRand(struct XorShift *xr, double *out_position);
extern void XorHollowSphereRand(struct XorShift *xr, double *out_position);
extern void XorSolidDiskRand(struct XorShift *xr, double *out_position);
extern void XorHollowDiskRand(struct XorShift *xr, double *out_position);
extern void XorSolidCubeRand(struct XorShift *xr, double *out_position);
extern void XorGaussianDiskRand(struct XorShift *xr, double *out_position);

extern double XorGaussianRand(struct XorShift *xr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

