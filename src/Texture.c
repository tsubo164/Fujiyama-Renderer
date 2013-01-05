/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Texture.h"
#include "FrameBufferIO.h"
#include "FrameBuffer.h"
#include "Mipmap.h"
#include "Vector.h"
#include "Box.h"
#include <stdlib.h>

static const float NO_TEXTURE_COLOR[] = {1, .63, .63};

struct Texture {
	struct FrameBuffer *fb;
	int width;
	int height;
	int databox[4];
	int viewbox[4];

	struct MipInput *mip;
	int lasttile[2];
};

struct Texture *TexNew(void)
{
	struct Texture *tex = (struct Texture *) malloc(sizeof(struct Texture));

	if (tex == NULL)
		return NULL;

	tex->mip = NULL;
	tex->lasttile[0] = -1;
	tex->lasttile[1] = -1;

	return tex;
}

void TexFree(struct Texture *tex)
{
	if (tex == NULL)
		return;

	FbFree(tex->fb);
	MipCloseInputFile(tex->mip);
	free(tex);
}

void TexLookup(struct Texture *tex, float u, float v, float *color)
{
	const float *pixel = NULL;
	float uv[2] = {0};

	int XNTILES, YNTILES;
	int xpxl, ypxl;
	int xtile, ytile;
	float tileuv[2] = {0};

	if (tex == NULL || tex->mip == NULL) {
		VEC3_COPY(color, NO_TEXTURE_COLOR);
		return;
	}

	uv[0] = u - floor(u);
	uv[1] = v - floor(v);

	XNTILES = tex->mip->xntiles;
	YNTILES = tex->mip->yntiles;

	tileuv[0] = uv[0] * XNTILES;
	tileuv[1] = (1-uv[1]) * YNTILES;

	xtile = (int) floor(tileuv[0]);
	ytile = (int) floor(tileuv[1]);

	if (xtile != tex->lasttile[0] || ytile != tex->lasttile[1]) {
		tex->mip->data = FbGetWritable(tex->fb, 0, 0, 0);
		MipReadTile(tex->mip, xtile, ytile);
		tex->lasttile[0] = xtile;
		tex->lasttile[1] = ytile;
	}

	xpxl = (int)( (tileuv[0] - floor(tileuv[0])) * 64);
	ypxl = (int)( (tileuv[1] - floor(tileuv[1])) * 64);
	pixel = FbGetReadOnly(tex->fb, xpxl, ypxl, 0);
	VEC3_SET(color, pixel[0], pixel[1], pixel[2]);
}

int TexLoadFile(struct Texture *tex, const char *filename)
{
	tex->mip = MipOpenInputFile(filename);
	if (tex->mip == NULL)
		return -1;

	tex->fb = FbNew();
	if (tex->fb == NULL)
		return -1;

	if (MipReadHeader(tex->mip))
		return -1;

	tex->width = tex->mip->width;
	tex->height = tex->mip->height;

	FbResize(tex->fb, tex->mip->tilesize, tex->mip->tilesize, tex->mip->nchannels);

	return 0;
}

void TexGetResolution(const struct Texture *tex, int *xres, int *yres)
{
	if (tex->mip == NULL) {
		*xres = 0;
		*yres = 0;
	}

	*xres = tex->mip->width;
	*yres = tex->mip->height;
}

