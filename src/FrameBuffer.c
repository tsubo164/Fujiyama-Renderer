/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "FrameBuffer.h"
#include "Numeric.h"
#include "Vector.h"
#include "Box.h"
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

struct FrameBuffer {
	float *buf;
	int width;
	int height;
	int nchannels;
};

static void free_buffer(struct FrameBuffer *fb);

struct FrameBuffer *FbNew(void)
{
	struct FrameBuffer *fb = (struct FrameBuffer *) malloc(sizeof(struct FrameBuffer));

	if (fb == NULL)
		return NULL;

	fb->buf = NULL;
	fb->width = 0;
	fb->height = 0;
	fb->nchannels = 0;

	return fb;
}

void FbFree(struct FrameBuffer *fb)
{
	if (fb == NULL)
		return;

	free_buffer(fb);
	free(fb);
}

int FbGetWidth(const struct FrameBuffer *fb)
{
	return fb->width;
}

int FbGetHeight(const struct FrameBuffer *fb)
{
	return fb->height;
}

int FbGetChannelCount(const struct FrameBuffer *fb)
{
	return fb->nchannels;
}

float *FbResize(struct FrameBuffer *fb, int width, int height, int nchannels)
{
	float *buftmp = NULL;
	int total_alloc = 0;

	assert(width >= 0);
	assert(height >= 0);
	assert(nchannels >= 0);

	total_alloc = width * height * nchannels;
	/* check overflow */
	if (total_alloc < 0)
		return NULL;

	if (total_alloc > 0) {
		buftmp = (float *) malloc(sizeof(float) * total_alloc);
		if (buftmp == NULL) {
			return NULL;
		}
	}

	/* successed to get new buffer then free old buffer if exists*/
	if (!FbIsEmpty(fb)) {
		free_buffer(fb);
	}

	/* commit */
	fb->buf = buftmp;
	fb->width = width;
	fb->height = height;
	fb->nchannels = nchannels;

	return fb->buf;
}

int FbComputeBounds(struct FrameBuffer *fb, int *bounds)
{
	const float *tmp = 0;
	int x, y;
	int xmin, ymin, xmax, ymax;

	/* XXX bounds can be computed only if alpha channel exists */
	if (fb->nchannels != 4)
		return -1;

	xmin =  INT_MAX;
	ymin =  INT_MAX;
	xmax = -INT_MAX;
	ymax = -INT_MAX;

	tmp = FbGetReadOnly(fb, 0, 0, 0);

	for (y = 0; y < fb->height; y++) {
		for (x = 0; x < fb->width; x++) {
			int i = y * fb->width * fb->nchannels + x * fb->nchannels;
			if (tmp[i + 0] > 0 ||
				tmp[i + 1] > 0 ||
				tmp[i + 2] > 0 ||
				tmp[i + 3] > 0) {
				xmin = MIN(xmin, x);
				ymin = MIN(ymin, y);
				xmax = MAX(xmax, x + 1);
				ymax = MAX(ymax, y + 1);
			}
		}
	}
	if (xmin ==  INT_MAX &&
		ymin ==  INT_MAX &&
		xmax == -INT_MAX &&
		ymax == -INT_MAX) {
		xmin = 0;
		ymin = 0;
		xmax = 0;
		ymax = 0;
	}

	BOX2_SET(bounds, xmin, ymin, xmax, ymax);
	return 0;
}

int FbIsEmpty(const struct FrameBuffer *fb)
{
	return fb->buf == NULL;
}

float *FbGetWritable(struct FrameBuffer *fb, int x, int y, int z)
{
	return fb->buf + y * fb->width * fb->nchannels + x * fb->nchannels + z;
}

const float *FbGetReadOnly(const struct FrameBuffer *fb, int x, int y, int z)
{
	return fb->buf + y * fb->width * fb->nchannels + x * fb->nchannels + z;
}

static void free_buffer(struct FrameBuffer *fb)
{
	free(fb->buf);
	fb->buf = NULL;
}

