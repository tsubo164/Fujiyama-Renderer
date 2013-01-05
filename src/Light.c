/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Light.h"
#include "ImportanceSampling.h"
#include "FrameBufferIO.h"
#include "FrameBuffer.h"
#include "Numeric.h"
#include "Texture.h"
#include "Random.h"
#include "Vector.h"
#include <stdlib.h>
#include <float.h>

struct Light {
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

	struct Texture *texture;
	/* TODO tmp solution for dome light data */
	struct DomeSample *dome_samples;

	/* functions */
	int (*GetSampleCount)(const struct Light *light);
	void (*GetSamples)(const struct Light *light,
			struct LightSample *samples, int max_samples);
	void (*Illuminate)(const struct Light *light,
			const struct LightSample *sample,
			const double *Ps, float *Cl);
	int (*Preprocess)(struct Light *light);
};

static int point_light_get_sample_count(const struct Light *light);
static void point_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples);
static void point_light_illuminate(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl);

static int grid_light_get_sample_count(const struct Light *light);
static void grid_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples);
static void grid_light_illuminate(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl);

static int sphere_light_get_sample_count(const struct Light *light);
static void sphere_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples);
static void sphere_light_illuminate(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl);

static int dome_light_get_sample_count(const struct Light *light);
static void dome_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples);
static void dome_light_illuminate(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl);
static int dome_light_preprocess(struct Light *light);

static int no_preprocess(struct Light *light);

/* TODO TEST */
#if 0
static int save_sample_points2(struct Light *light);
#endif

struct Light *LgtNew(int light_type)
{
	struct Light *light = (struct Light *) malloc(sizeof(struct Light));

	if (light == NULL)
		return NULL;

	XfmInitTransformSampleList(&light->transform_samples);

	VEC3_SET(light->color, 1, 1, 1);
	light->intensity = 1;

	light->type = light_type;
	light->double_sided = 0;
	light->sample_count = 16;
	light->sample_intensity = light->intensity / light->sample_count;

	light->texture = NULL;
	light->dome_samples = NULL;
	XorInit(&light->xr);

	switch (light->type) {
	case LGT_POINT:
		light->GetSampleCount = point_light_get_sample_count;
		light->GetSamples     = point_light_get_samples;
		light->Illuminate     = point_light_illuminate;
		light->Preprocess     = no_preprocess;
		break;
	case LGT_GRID:
		light->GetSampleCount = grid_light_get_sample_count;
		light->GetSamples     = grid_light_get_samples;
		light->Illuminate     = grid_light_illuminate;
		break;
	case LGT_SPHERE:
		light->GetSampleCount = sphere_light_get_sample_count;
		light->GetSamples     = sphere_light_get_samples;
		light->Illuminate     = sphere_light_illuminate;
		light->Preprocess     = no_preprocess;
		break;
	case LGT_DOME:
		light->GetSampleCount = dome_light_get_sample_count;
		light->GetSamples     = dome_light_get_samples;
		light->Illuminate     = dome_light_illuminate;
		light->Preprocess     = dome_light_preprocess;
		break;
	default:
		/* TODO should abort()? */
		light->type = LGT_POINT;
		light->GetSampleCount = point_light_get_sample_count;
		light->GetSamples     = point_light_get_samples;
		light->Illuminate     = point_light_illuminate;
		light->Preprocess     = no_preprocess;
		break;
	}

	return light;
}

void LgtFree(struct Light *light)
{
	if (light == NULL)
		return;

	if (light->dome_samples != NULL)
		free(light->dome_samples);

	free(light);
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

void LgtSetTexture(struct Light *light, struct Texture *texture)
{
	light->texture = texture;
}

void LgtSetTranslate(struct Light *light,
		double tx, double ty, double tz, double time)
{
	XfmPushTranslateSample(&light->transform_samples, tx, ty, tz, time);
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
void LgtIlluminate(const struct LightSample *sample,
		const double *Ps, float *Cl)
{
	const struct Light *light = sample->light;
	light->Illuminate(light, sample, Ps, Cl);
}

int LgtPreprocess(struct Light *light)
{
	return light->Preprocess(light);
}

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
	XfmLerpTransformSample(&light->transform_samples, 0, &transform_interp);

	VEC3_COPY(samples[0].P, transform_interp.translate);
	VEC3_SET(samples[0].N, 0, 0, 0);
	samples[0].light = light;
}

static void point_light_illuminate(const struct Light *light,
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
	double N_sample[] = {0, 1, 0};
	int i;

	/* TODO time sampling */
	XfmLerpTransformSample(&light->transform_samples, 0, &transform_interp);

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

static void grid_light_illuminate(const struct Light *light,
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
	XfmLerpTransformSample(&light->transform_samples, 0, &transform_interp);

	nsamples = MIN(nsamples, max_samples);
	for (i = 0; i < nsamples; i++) {
		struct XorShift *mutable_xr = (struct XorShift *) &light->xr;
		double P_sample[] = {0, 0, 0};
		double N_sample[] = {0, 0, 0};

		XorHollowSphereRand(mutable_xr, P_sample);
		VEC3_COPY(N_sample, P_sample);

		XfmTransformPoint(&transform_interp, P_sample);
		XfmTransformVector(&transform_interp, N_sample);
		VEC3_NORMALIZE(N_sample);

		VEC3_COPY(samples[i].P, P_sample);
		VEC3_COPY(samples[i].N, N_sample);
		samples[i].light = light;
	}
}

static void sphere_light_illuminate(const struct Light *light,
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

/* dome light */
static int dome_light_get_sample_count(const struct Light *light)
{
	return light->sample_count;
}

static void dome_light_get_samples(const struct Light *light,
		struct LightSample *samples, int max_samples)
{
	struct Transform transform_interp;
	int nsamples = LgtGetSampleCount(light);
	int i;

	/* TODO time sampling */
	XfmLerpTransformSample(&light->transform_samples, 0, &transform_interp);

	nsamples = MIN(nsamples, max_samples);
	for (i = 0; i < nsamples; i++) {
		const struct DomeSample *dome_sample = &light->dome_samples[i];
		double P_sample[] = {0, 0, 0};
		double N_sample[] = {0, 0, 0};

		P_sample[0] = dome_sample->dir[0] * FLT_MAX;
		P_sample[1] = dome_sample->dir[1] * FLT_MAX;
		P_sample[2] = dome_sample->dir[2] * FLT_MAX;
		N_sample[0] = -1 * dome_sample->dir[0];
		N_sample[1] = -1 * dome_sample->dir[1];
		N_sample[2] = -1 * dome_sample->dir[2];

		/* TODO cancel translate and scale */
		XfmTransformPoint(&transform_interp, P_sample);
		XfmTransformVector(&transform_interp, N_sample);

		VEC3_COPY(samples[i].P, P_sample);
		VEC3_COPY(samples[i].N, N_sample);
		VEC3_COPY(samples[i].color, dome_sample->color);
		samples[i].light = light;
	}
}

static void dome_light_illuminate(const struct Light *light,
		const struct LightSample *sample,
		const double *Ps, float *Cl)
{
	Cl[0] = light->sample_intensity * sample->color[0];
	Cl[1] = light->sample_intensity * sample->color[1];
	Cl[2] = light->sample_intensity * sample->color[2];
}

static int dome_light_preprocess(struct Light *light)
{
	const int NSAMPLES = LgtGetSampleCount(light);
	int XRES = 200;
	int YRES = 100;
	int i;

	if (light->dome_samples != NULL) {
		free(light->dome_samples);
	}

	light->dome_samples =
			(struct DomeSample *) malloc(sizeof(struct DomeSample) * NSAMPLES);
	for (i = 0; i < NSAMPLES; i++) {
		struct DomeSample *sample = &light->dome_samples[i];
		VEC2_SET(sample->uv, 1./NSAMPLES, 1./NSAMPLES);
		VEC3_SET(sample->color, 1, .63, .63);
		VEC3_SET(sample->dir, 1./NSAMPLES, 1, 1./NSAMPLES);
		VEC3_NORMALIZE(sample->dir);
	}

	if (light->texture == NULL) {
		/* TODO should be an error? */
		return 0;
	}

	TexGetResolution(light->texture, &XRES, &YRES);
	/* TODO parameteraize */
	XRES /= 8;
	YRES /= 8;

	if (0) {
		ImportanceSampling(light->texture, 0,
				XRES, YRES,
				light->dome_samples, NSAMPLES);
	} else if (1) {
		StratifiedImportanceSampling(light->texture, 0,
				XRES, YRES,
				light->dome_samples, NSAMPLES);
	} else {
		StructuredImportanceSampling(light->texture, 0,
				XRES, YRES,
				light->dome_samples, NSAMPLES);
	}

	return 0;
}

static int no_preprocess(struct Light *light)
{
	/* do nothing */
	return 0;
}

/* TODO TEST */
#if 0
static int save_sample_points2(struct Light *light)
{
	const char *filename = "../sample_points.fb";
	struct FrameBuffer *fb = NULL;
	const int NSAMPLES = LgtGetSampleCount(light);
	int XRES = 1000 / 4;
	int YRES = 500 / 4;
	int err = 0;
	int i;

	/* TODO TEST */
	struct Timer timer;
	struct Elapse elapse;

	if (light->texture == NULL)
		return -1;

	TexGetResolution(light->texture, &XRES, &YRES);
	XRES /= 8;
	YRES /= 8;
	/*
	XRES /= 32;
	YRES /= 32;
	*/

	fb = FbNew();
	if (fb == NULL)
		return -1;

	FbResize(fb, XRES, YRES, 4);

	light->dome_samples =
			(struct DomeSample *) malloc(sizeof(struct DomeSample) * NSAMPLES);

	TimerStart(&timer);

	if (0) {
		ImportanceSampling(light->texture, 0,
				XRES, YRES,
				light->dome_samples, NSAMPLES);
	} else if (1) {
		StratifiedImportanceSampling(light->texture, 0,
				XRES, YRES,
				light->dome_samples, NSAMPLES);
	} else {
		StructuredImportanceSampling(light->texture, 0,
				XRES, YRES,
				light->dome_samples, NSAMPLES);
	}

	elapse = TimerElapsed(&timer);
	printf("Done: %dh %dm %gs\n", elapse.hour, elapse.min, elapse.sec);

	{
		int x, y;

		for (y = 0; y < YRES; y++) {
			for (x = 0; x < XRES; x++) {
				struct Color4 C_fb = {0};
				float C_tex[3] = {0};

				TexLookup(light->texture,
						(.5 + x) / XRES,
						1 - (.5 + y) / YRES,
						C_tex);

				C_fb.r = C_tex[0];
				C_fb.g = C_tex[1];
				C_fb.b = C_tex[2];
				C_fb.a = 0;

				FbSetColor4(fb, x, y, 0, &C_fb);
			}
		}
	}

	for (i = 0; i < NSAMPLES; i++) {
		const float *uv = light->dome_samples[i].uv;
		const struct Color4 C_sample = {0, 1, 0, 1};
		const int x = (int) (uv[0] * XRES);
		const int y = (int) ((1 - uv[1]) * YRES);
		FbSetColor4(fb, x, y, 0, &C_sample);
	}

	err = FbSaveCroppedData(fb, filename);
	if (err) {
		FbFree(fb);
		return -1;
	}
	FbFree(fb);

	return 0;
}
#endif

