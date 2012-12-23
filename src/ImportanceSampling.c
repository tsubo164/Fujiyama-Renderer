/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "ImportanceSampling.h"
#include "Numeric.h"
#include "Texture.h"
#include "Random.h"
#include "Vector.h"
#include <stdlib.h>

static void make_histgram(struct Texture *texture,
		int sample_xres, int sample_yres, double *histgram);
static int lookup_histgram(const double *histgram, int pixel_count, double key_value);
static void index_to_uv(int xres, int yres, int index, float *uv);
static void uv_to_dir(float u, float v, double *dir);

int ImportanceSampling(struct Texture *texture, int seed,
		int sample_xres, int sample_yres,
		struct DomeSample *dome_samples, int sample_count)
{
	const int NPIXELS = sample_xres * sample_yres;
	double *histgram = (double *) malloc(sizeof(double) * NPIXELS);
	char *picked = (char *) malloc(sizeof(char) * NPIXELS);

	struct XorShift xr;
	double sum = 0;
	int i;

	for (i = 0; i < NPIXELS; i++) {
		picked[i] = 0;
	}

	make_histgram(texture, sample_xres, sample_yres, histgram);
	sum = histgram[NPIXELS-1];

	XorInit(&xr);
	for (i = 0; i < sample_count; i++) {
		for (;;) {
			const double rand = sum * XorNextFloat01(&xr);
			const int index = lookup_histgram(histgram, NPIXELS, rand);
			struct DomeSample sample = {{0}};

			if (picked[index] == 1) {
				/*
				continue;
				*/
			}

			index_to_uv(sample_xres, sample_yres, index, sample.uv);
			uv_to_dir(sample.uv[0], sample.uv[1], sample.dir);
			TexLookup(texture, sample.uv[0], sample.uv[1], sample.color);

			dome_samples[i] = sample;
			picked[index] = 1;
			break;
		}
	}

	free(picked);
	free(histgram);
	return 0;
}

int StratifiedImportanceSampling(struct Texture *texture, int seed,
		int sample_xres, int sample_yres,
		struct DomeSample *dome_samples, int sample_count)
{
	const int NPIXELS = sample_xres * sample_yres;
	double *histgram = (double *) malloc(sizeof(double) * NPIXELS);
	char *picked = (char *) malloc(sizeof(char) * NPIXELS);

	struct XorShift xr;
	double sum = 0;
	int i;

	for (i = 0; i < NPIXELS; i++) {
		picked[i] = 0;
	}

	make_histgram(texture, sample_xres, sample_yres, histgram);
	sum = histgram[NPIXELS-1];

	XorInit(&xr);
	for (i = 0; i < seed; i++) {
		XorNextFloat01(&xr);
	}

	for (i = 0; i < sample_count; i++) {
		for (;;) {
			const double rand = sum * ((i + XorNextFloat01(&xr)) / sample_count);
			const int index = lookup_histgram(histgram, NPIXELS, rand);
			struct DomeSample sample = {{0}};

			if (picked[index] == 1) {
				/*
				continue;
				*/
			}

			index_to_uv(sample_xres, sample_yres, index, sample.uv);
			uv_to_dir(sample.uv[0], sample.uv[1], sample.dir);
			TexLookup(texture, sample.uv[0], sample.uv[1], sample.color);

			dome_samples[i] = sample;
			picked[index] = 1;
			break;
		}
	}

	free(picked);
	free(histgram);
	return 0;
}

static void make_histgram(struct Texture *texture,
		int sample_xres, int sample_yres, double *histgram)
{
	double sum = 0;
	int index = 0;
	int x, y;

	for (y = 0; y < sample_yres; y++) {
		for (x = 0; x < sample_xres; x++) {
			float color[3] = {0};
			float uv[2] = {0};

			index_to_uv(sample_xres, sample_yres, index, uv);
			TexLookup(texture, uv[0], uv[1], color);

			sum += .2989 * color[0] + .5866 * color[1] + .1145 * color[2];
			histgram[index] = sum;
			index++;
		}
	}
}

static int lookup_histgram(const double *histgram, int pixel_count, double key_value)
{
	int i;

	for (i = 0; i < pixel_count; i++) {
		if (key_value < histgram[i]) {
			return i;
		}
	}
	return -1;
}

static void index_to_uv(int xres, int yres, int index, float *uv)
{
	const int x = (int) index % xres;
	const int y = (int) index / xres;

	uv[0] = (.5 + x) / xres;
	uv[1] = 1. - ((.5 + y) / yres);
}

static void uv_to_dir(float u, float v, double *dir)
{
	const double phi = 2 * N_PI * u;
	const double theta = N_PI * (v - .5);
	const double r = cos(theta);

	dir[0] = r * sin(phi);
	dir[1] = sin(theta);
	dir[2] = r * cos(phi);
}

