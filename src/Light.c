/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Light.h"
#include "Numeric.h"
#include "Vector.h"
#include <stdlib.h>

/* TODO TEST */
#include "Random.h"

struct LightFunctions {
	int (*GetSampleCount)(const struct Light *light);
	void (*GetSamples)(const struct Light *light,
			struct LightSample *samples, int max_samples);
	void (*IlluminateFromSample)(const struct Light *light,
			const struct LightSample *sample,
			const double *Ps, float *Cl);
};

struct Light {
	double position[3];
	float color[3];
	float intensity;

	/* TODO TEST */
	/* transformation properties */
	/*
	struct TransformSampleList transform_samples;
	*/

	int type;
	int sample_count;
	float sample_intensity;
	struct LightFunctions vtbl;
};

static int point_light_get_sample_count(const struct Light *light)
{
	return 1;
}
static void point_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples)
{
	if (max_samples == 0)
		return;
	VEC3_COPY(samples[0].position, light->position);
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

static int geometry_light_get_sample_count(const struct Light *light)
{
	return light->sample_count;
}
static void geometry_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples)
{
	int nsamples = LgtGetSampleCount(light);
	int i;

	/* TODO temp solution */
	static struct XorShift xr;
	static int init_xr = 0;
	if (init_xr == 0) {
		XorInit(&xr);
		init_xr = 1;
	}

	nsamples = MIN(nsamples, max_samples);
	for (i = 0; i < nsamples; i++) {
		const double x = XorNextFloat01(&xr) * 10;
		const double z = XorNextFloat01(&xr) * 10;
		double p[] = {0, 10, 0};
		p[0] = x;
		p[2] = z;

		VEC3_COPY(samples[i].position, p);
		samples[i].light = light;
	}
}
static void geometry_light_illuminate_from_sample(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl)
{
	Cl[0] = light->sample_intensity * light->color[0];
	Cl[1] = light->sample_intensity * light->color[1];
	Cl[2] = light->sample_intensity * light->color[2];
}

static const struct LightFunctions point_light_functions = {
	point_light_get_sample_count,
	point_light_get_samples,
	point_light_illuminate_from_sample
};

static const struct LightFunctions geometry_light_functions = {
	geometry_light_get_sample_count,
	geometry_light_get_samples,
	geometry_light_illuminate_from_sample
};

struct Light *LgtNew(int light_type)
{
	struct Light *light;

	light = (struct Light *) malloc(sizeof(struct Light));
	if (light == NULL)
		return NULL;

	VEC3_SET(light->position, 0, 0, 0);
	VEC3_SET(light->color, 1, 1, 1);
	light->intensity = 1;

	/* TODO TEST */
	light->type = light_type;
	light->sample_count = 16;
	light->sample_intensity = light->intensity / light->sample_count;
	/*
	light->vtbl = point_light_functions;
	light->vtbl = geometry_light_functions;
	*/
	switch (light->type) {
	case LGT_POINT:
		light->vtbl = point_light_functions;
		break;
	case LGT_GEOMETRY:
		light->vtbl = geometry_light_functions;
		break;
	default:
		/* TODO should abort()? */
		light->type = LGT_POINT;
		light->vtbl = point_light_functions;
		break;
	}

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
	light->position[0] = xpos;
	light->position[1] = ypos;
	light->position[2] = zpos;
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

const double *LgtGetPosition(const struct Light *light)
{
	return light->position;
}

void LgtGetSamples(const struct Light *light,
		struct LightSample *samples, int max_samples)
{
	light->vtbl.GetSamples(light, samples, max_samples);
}

int LgtGetSampleCount(const struct Light *light)
{
	return light->vtbl.GetSampleCount(light);
}

/* TODO should have struct Light *light parameter? */
void LgtIlluminateFromSample(const struct LightSample *sample,
		const double *Ps, float *Cl)
{
	const struct Light *light = sample->light;
	light->vtbl.IlluminateFromSample(light, sample, Ps, Cl);
}

void LgtIlluminate(const struct Light *light, const double *Ps, float *Cl)
{
	Cl[0] = light->intensity * light->color[0];
	Cl[1] = light->intensity * light->color[1];
	Cl[2] = light->intensity * light->color[2];
}

