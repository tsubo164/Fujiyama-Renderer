/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Light.h"
#include "Numeric.h"
#include "Random.h"
#include "Vector.h"
#include <stdlib.h>

struct Light {
	double position[3];
	float color[3];
	float intensity;

	/* transformation properties */
	struct TransformSampleList transform_samples;

	/* rng */
	struct XorShift xr;

	int type;
	int double_sided;
	int sample_count;
	float sample_intensity;

	/* functions */
	int (*GetSampleCount)(const struct Light *light);
	void (*GetSamples)(const struct Light *light,
			struct LightSample *samples, int max_samples);
	void (*IlluminateFromSample)(const struct Light *light,
			const struct LightSample *sample,
			const double *Ps, float *Cl);
};

static int point_light_get_sample_count(const struct Light *light);
static void point_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples);
static void point_light_illuminate_from_sample(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl);

static int grid_light_get_sample_count(const struct Light *light);
static void grid_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples);
static void grid_light_illuminate_from_sample(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl);

static int sphere_light_get_sample_count(const struct Light *light);
static void sphere_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples);
static void sphere_light_illuminate_from_sample(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl);

struct Light *LgtNew(int light_type)
{
	struct Light *light;

	light = (struct Light *) malloc(sizeof(struct Light));
	if (light == NULL)
		return NULL;

	XfmInitTransformSampleList(&light->transform_samples);

	VEC3_SET(light->position, 0, 0, 0);
	VEC3_SET(light->color, 1, 1, 1);
	light->intensity = 1;

	light->type = light_type;
	light->double_sided = 0;
	light->sample_count = 16;
	light->sample_intensity = light->intensity / light->sample_count;

	switch (light->type) {
	case LGT_POINT:
		light->GetSampleCount       = point_light_get_sample_count;
		light->GetSamples           = point_light_get_samples;
		light->IlluminateFromSample = point_light_illuminate_from_sample;
		break;
	case LGT_GRID:
		light->GetSampleCount       = grid_light_get_sample_count;
		light->GetSamples           = grid_light_get_samples;
		light->IlluminateFromSample = grid_light_illuminate_from_sample;
		break;
	case LGT_SPHERE:
		light->GetSampleCount       = sphere_light_get_sample_count;
		light->GetSamples           = sphere_light_get_samples;
		light->IlluminateFromSample = sphere_light_illuminate_from_sample;
		break;
	default:
		/* TODO should abort()? */
		light->type = LGT_POINT;
		light->GetSampleCount       = point_light_get_sample_count;
		light->GetSamples           = point_light_get_samples;
		light->IlluminateFromSample = point_light_illuminate_from_sample;
		break;
	}

	XorInit(&light->xr);
	return light;
}

void LgtFree(struct Light *light)
{
	if (light == NULL)
		return;
	free(light);
}

void LgtSetPosition(struct Light *light, double xpos, double ypos, double zpos)
{
	/* TODO TEST temp no sample time */
	LgtSetTranslate(light, xpos, ypos, zpos, 0);
}

void LgtSetColor(struct Light *light, float r, float g, float b)
{
	light->color[0] = r;
	light->color[1] = g;
	light->color[2] = b;
}

void LgtSetIntensity(struct Light *light, double intensity)
{
	light->intensity = intensity;
	/* TODO temp */
	light->sample_intensity = light->intensity / light->sample_count;
}

void LgtSetSampleCount(struct Light *light, int sample_count)
{
	light->sample_count = MAX(sample_count, 1);
	/* TODO temp */
	light->sample_intensity = light->intensity / light->sample_count;
}

void LgtSetDoubleSided(struct Light *light, int on_or_off)
{
	light->double_sided = (on_or_off != 0);
}

/* TODO obsolete */
#if 0
const double *LgtGetPosition(const struct Light *light)
{
	return light->position;
}
#endif

void LgtSetTranslate(struct Light *light,
		double tx, double ty, double tz, double time)
{
	XfmPushTranslateSample(&light->transform_samples, tx, ty, tz, time);

	light->position[0] = tx;
	light->position[1] = ty;
	light->position[2] = tz;
}

void LgtSetRotate(struct Light *light,
		double rx, double ry, double rz, double time)
{
	XfmPushRotateSample(&light->transform_samples, rx, ry, rz, time);
}

void LgtSetScale(struct Light *light,
		double sx, double sy, double sz, double time)
{
	XfmPushScaleSample(&light->transform_samples, sx, sy, sz, time);
}

void LgtSetTransformOrder(struct Light *light, int order)
{
	XfmSetSampleTransformOrder(&light->transform_samples, order);
}

void LgtSetRotateOrder(struct Light *light, int order)
{
	XfmSetSampleRotateOrder(&light->transform_samples, order);
}

void LgtGetSamples(const struct Light *light,
		struct LightSample *samples, int max_samples)
{
	light->GetSamples(light, samples, max_samples);
}

int LgtGetSampleCount(const struct Light *light)
{
	return light->GetSampleCount(light);
}

/* TODO should have struct Light *light parameter? */
void LgtIlluminateFromSample(const struct LightSample *sample,
		const double *Ps, float *Cl)
{
	const struct Light *light = sample->light;
	light->IlluminateFromSample(light, sample, Ps, Cl);
}

/* TODO obsolete */
#if 0
void LgtIlluminate(const struct Light *light, const double *Ps, float *Cl)
{
	Cl[0] = light->intensity * light->color[0];
	Cl[1] = light->intensity * light->color[1];
	Cl[2] = light->intensity * light->color[2];
}
#endif

/* point light */
static int point_light_get_sample_count(const struct Light *light)
{
	return 1;
}

static void point_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples)
{
	struct Transform transform_interp;

	if (max_samples == 0)
		return;

	/* TODO time sampling */
	XfmLerpTransformSample2(&light->transform_samples, 0, &transform_interp);

	VEC3_COPY(samples[0].P, transform_interp.translate);
	VEC3_SET(samples[0].N, 0, 0, 0);
	samples[0].light = light;
}

static void point_light_illuminate_from_sample(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl)
{
	Cl[0] = light->intensity * light->color[0];
	Cl[1] = light->intensity * light->color[1];
	Cl[2] = light->intensity * light->color[2];
}

/* grid light */
static int grid_light_get_sample_count(const struct Light *light)
{
	return light->sample_count;
}

static void grid_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples)
{
	struct Transform transform_interp;
	int nsamples = LgtGetSampleCount(light);
	int i;
	double N_sample[] = {0, 1, 0};

	/* TODO time sampling */
	XfmLerpTransformSample2(&light->transform_samples, 0, &transform_interp);

	XfmTransformVector(&transform_interp, N_sample);
	VEC3_NORMALIZE(N_sample);

	nsamples = MIN(nsamples, max_samples);
	for (i = 0; i < nsamples; i++) {
		struct XorShift *mutable_xr = (struct XorShift *) &light->xr;
		const double x = (XorNextFloat01(mutable_xr) - .5);
		const double z = (XorNextFloat01(mutable_xr) - .5);
		double P_sample[] = {0, 0, 0};
		P_sample[0] = x;
		P_sample[2] = z;

		XfmTransformPoint(&transform_interp, P_sample);

		VEC3_COPY(samples[i].P, P_sample);
		VEC3_COPY(samples[i].N, N_sample);
		samples[i].light = light;
	}
}

static void grid_light_illuminate_from_sample(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl)
{
	double Ln[3] = {0, 0, 0};
	double dot = 0;

	Ln[0] = Ps[0] - sample->P[0];
	Ln[1] = Ps[1] - sample->P[1];
	Ln[2] = Ps[2] - sample->P[2];

	VEC3_NORMALIZE(Ln);

	dot = VEC3_DOT(Ln, sample->N);
	if (light->double_sided) {
		dot = ABS(dot);
	} else {
		dot = MAX(dot, 0);
	}

	Cl[0] = light->sample_intensity * light->color[0];
	Cl[1] = light->sample_intensity * light->color[1];
	Cl[2] = light->sample_intensity * light->color[2];

	Cl[0] *= dot;
	Cl[1] *= dot;
	Cl[2] *= dot;
}

/* sphere light */
static int sphere_light_get_sample_count(const struct Light *light)
{
	return light->sample_count;
}

static void sphere_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples)
{
	struct Transform transform_interp;
	int nsamples = LgtGetSampleCount(light);
	int i;

	/* TODO time sampling */
	XfmLerpTransformSample2(&light->transform_samples, 0, &transform_interp);

	nsamples = MIN(nsamples, max_samples);
	for (i = 0; i < nsamples; i++) {
		struct XorShift *mutable_xr = (struct XorShift *) &light->xr;
		double P_sample[] = {0, 0, 0};
		double N_sample[] = {0, 0, 0};
		P_sample[0] = (XorNextFloat01(mutable_xr) - .5);
		P_sample[1] = (XorNextFloat01(mutable_xr) - .5);
		P_sample[2] = (XorNextFloat01(mutable_xr) - .5);
		VEC3_NORMALIZE(P_sample);
		VEC3_COPY(N_sample, P_sample);

		XfmTransformPoint(&transform_interp, P_sample);
		XfmTransformVector(&transform_interp, N_sample);
		VEC3_NORMALIZE(N_sample);

		VEC3_COPY(samples[i].P, P_sample);
		VEC3_COPY(samples[i].N, N_sample);
		samples[i].light = light;
	}
}

static void sphere_light_illuminate_from_sample(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl)
{
	double Ln[3] = {0, 0, 0};
	double dot = 0;

	Ln[0] = Ps[0] - sample->P[0];
	Ln[1] = Ps[1] - sample->P[1];
	Ln[2] = Ps[2] - sample->P[2];

	VEC3_NORMALIZE(Ln);

	dot = VEC3_DOT(Ln, sample->N);

	if (dot > 0) {
		Cl[0] = light->sample_intensity * light->color[0];
		Cl[1] = light->sample_intensity * light->color[1];
		Cl[2] = light->sample_intensity * light->color[2];
	} else {
		Cl[0] = 0;
		Cl[1] = 0;
		Cl[2] = 0;
	}
}

