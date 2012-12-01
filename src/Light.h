/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef LIGHT_H
#define LIGHT_H

#include "Transform.h"

#ifdef __cplusplus
extern "C" {
#endif

enum LightType {
	LGT_POINT = 0,
	LGT_GRID,
	LGT_SPHERE
};

struct Light;

struct LightSample {
	const struct Light *light;
	double P[3];
	double N[3];
};

extern struct Light *LgtNew(int light_type);
extern void LgtFree(struct Light *light);

extern void LgtSetColor(struct Light *light, float r, float g, float b);
extern void LgtSetIntensity(struct Light *light, double intensity);
extern void LgtSetSampleCount(struct Light *light, int sample_count);
extern void LgtSetDoubleSided(struct Light *light, int on_or_off);

/* transformation */
extern void LgtSetTranslate(struct Light *light,
		double tx, double ty, double tz, double time);
extern void LgtSetRotate(struct Light *light,
		double rx, double ry, double rz, double time);
extern void LgtSetScale(struct Light *light,
		double sx, double sy, double sz, double time);
extern void LgtSetTransformOrder(struct Light *light, int order);
extern void LgtSetRotateOrder(struct Light *light, int order);

/* samples */
extern void LgtGetSamples(const struct Light *light,
		struct LightSample *samples, int max_samples);
extern int LgtGetSampleCount(const struct Light *light);
extern void LgtIlluminate(const struct LightSample *sample,
		const double *Ps, float *Cl);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

