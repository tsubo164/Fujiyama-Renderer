/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef LIGHT_H
#define LIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

enum LightType {
	LGT_POINT = 0,
	LGT_GEOMETRY
};

struct Light;

struct LightSample {
	const struct Light *light;
	double position[3];
};

extern struct Light *LgtNew(int light_type);
extern void LgtFree(struct Light *light);

extern void LgtSetPosition(struct Light *light, double xpos, double ypos, double zpos);
extern void LgtSetColor(struct Light *light, float r, float g, float b);
extern void LgtSetIntensity(struct Light *light, double intensity);
extern void LgtSetSampleCount(struct Light *light, int sample_count);

extern const double *LgtGetPosition(const struct Light *light);

/* TODO TEST */
extern void LgtGetSamples(const struct Light *light,
		struct LightSample *samples, int max_samples);
extern int LgtGetSampleCount(const struct Light *light);
extern void LgtIlluminateFromSample(const struct LightSample *sample,
		const double *Ps, float *Cl);

extern void LgtIlluminate(const struct Light *light, const double *Ps, float *Cl);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

