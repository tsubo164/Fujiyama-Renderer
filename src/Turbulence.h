/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TURBULENCE_H
#define TURBULENCE_H

#ifdef __cplusplus
extern "C" {
#endif

struct Turbulence;

extern struct Turbulence *TrbNew(void);
extern void TrbFree(struct Turbulence *turb);

extern void TrbSetAmplitude(struct Turbulence *turb, double x, double y, double z);
extern void TrbSetFrequency(struct Turbulence *turb, double x, double y, double z);
extern void TrbSetOffset(struct Turbulence *turb, double x, double y, double z);
extern void TrbSetLacunarity(struct Turbulence *turb, double lacunarity);
extern void TrbSetGain(struct Turbulence *turb, double gain);
extern void TrbSetOctaves(struct Turbulence *turb, int octaves);

extern double TrbEvaluate(struct Turbulence *turb, double *position);
extern void TrbEvaluate3(struct Turbulence *turb, double *position, double *out_noise);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

