/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef LIGHT_H
#define LIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

struct Light;

extern struct Light *LgtNew(const char *type);
extern void LgtFree(struct Light *light);

extern void LgtSetPosition(struct Light *light, double xpos, double ypos, double zpos);
extern void LgtSetColor(struct Light *light, float r, float g, float b);
extern void LgtSetIntensity(struct Light *light, double intensity);
extern const double *LgtGetPosition(const struct Light *light);

extern void LgtIlluminate(const struct Light *light, const double *Ps, float *Cl);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

