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

extern struct Curve *CrvNew(void);
extern void CrvFree(struct Curve *light);

extern void CrvSetupAccelerator(const struct Curve *curve, struct Accelerator *acc);

#if 0
extern void CrvSetPosition(struct Curve *light, double xpos, double ypos, double zpos);
extern void CrvSetColor(struct Curve *light, float r, float g, float b);
extern void CrvSetIntensity(struct Curve *light, double intensity);
extern const double *CrvGetPosition(const struct Curve *light);

extern void CrvIlluminate(const struct Curve *light, const double *Ps, float *Cl);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

