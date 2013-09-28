/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TURBULENCE_H
#define TURBULENCE_H

#ifdef __cplusplus
extern "C" {
#endif

struct Turbulence;
struct Vector;

extern struct Turbulence *TrbNew(void);
extern void TrbFree(struct Turbulence *turbulence);

extern void TrbSetAmplitude(struct Turbulence *turbulence, double x, double y, double z);
extern void TrbSetFrequency(struct Turbulence *turbulence, double x, double y, double z);
extern void TrbSetOffset(struct Turbulence *turbulence, double x, double y, double z);
extern void TrbSetLacunarity(struct Turbulence *turbulence, double lacunarity);
extern void TrbSetGain(struct Turbulence *turbulence, double gain);
extern void TrbSetOctaves(struct Turbulence *turbulence, int octaves);

extern double TrbEvaluate(const struct Turbulence *turbulence, const struct Vector *position);
extern void TrbEvaluate3d(const struct Turbulence *turbulence, const struct Vector *position,
    struct Vector *out_noise);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

