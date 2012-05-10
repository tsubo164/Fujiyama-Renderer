/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "FrameBufferViewer.h"
#include "FrameBufferIO.h"
#include "FrameBuffer.h"
#include "Numeric.h"
#include "Mipmap.h"
#include "String.h"
#include "Box.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <GL/gl.h>
#include <GL/glu.h>

enum DisplayChannels {
	DISP_RGB = 0,
	DISP_R,
	DISP_G,
	DISP_B,
	DISP_A
};

struct FrameBufferViewer {
	struct FrameBuffer *fb;
	char filename[1024];

	int is_gl_initialized;

	int win_width;
	int win_height;
	int disp_chans;

	MouseButton pressbutton;
	int xpresspos;
	int ypresspos;

	float scale;
	float exponent;
	float lockexponent;
	float dist_per_pixel;
	int xoffset;
	int yoffset;
	int xlockoffset;
	int ylockoffset;

	int databox[4];
	int viewbox[4];

	int tilesize;
	int draw_tile;
};

static int load_fb(struct FrameBufferViewer *v);
static int load_mip(struct FrameBufferViewer *v);
static const char *file_extension(const char *filename);

static void clear_image_viewer(struct FrameBufferViewer *v);
static void initialize_gl(struct FrameBufferViewer *v);
static void set_to_home_position(struct FrameBufferViewer *v);

void GlDrawTileGuide(int width, int height, int tilesize);

static void clear_image_viewer(struct FrameBufferViewer *v)
{
	v->fb = NULL;

	v->is_gl_initialized = 0;

	v->win_width = 0;
	v->win_height = 0;
	v->disp_chans = DISP_RGB;

	v->pressbutton = MB_NONE;

	set_to_home_position(v);

	BOX2_SET(v->databox, 0, 0, 0, 0);
	BOX2_SET(v->viewbox, 0, 0, 0, 0);
	v->tilesize = 0;
	v->draw_tile = 1;
}

struct FrameBufferViewer *FbvNewViewer(void)
{
	struct FrameBufferViewer *v;
	v = (struct FrameBufferViewer *) malloc(sizeof(struct FrameBufferViewer));

	if (v == NULL)
		return NULL;
	
	clear_image_viewer(v);
	return v;
}

void FbvFreeViewer(struct FrameBufferViewer *v)
{
	if (v == NULL)
		return;

	if (v->fb != NULL)
		FbFree(v->fb);

	free(v);
}

void FbvDraw(struct FrameBufferViewer *v)
{
	int xmove = 0.f;
	int ymove = 0.f;
	int xviewsize = v->viewbox[2] - v->viewbox[0];
	int yviewsize = v->viewbox[3] - v->viewbox[1];

	if (!v->is_gl_initialized) {
		initialize_gl(v);
		v->is_gl_initialized = 1;
	}

	glClearColor(.2f, .2f, .2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (v->fb == NULL)
		return;

	xmove = v->scale * v->xoffset; 
	ymove = v->scale * v->yoffset; 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(xmove, ymove, 0.f);
	glScalef(v->scale, v->scale, 1.f);
	glTranslatef(-.5 * xviewsize, -.5 * yviewsize, 0.f); 

	/* render background */
	glColor3f(0.f, 0.f, 0.f);
	glBegin(GL_QUADS);
		glVertex3f(v->viewbox[0], v->viewbox[1], 0.f);
		glVertex3f(v->viewbox[2], v->viewbox[1], 0.f);
		glVertex3f(v->viewbox[2], v->viewbox[3], 0.f);
		glVertex3f(v->viewbox[0], v->viewbox[3], 0.f);
	glEnd();

	/* render framebuffer */
	glTranslatef(0.f, 0.f, 0.1f); 
	glColor3f(1.f, 1.f, 1.f);
	if (!FbIsEmpty(v->fb)) {
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glTexCoord2f(0.f, 1.f);
			glVertex3f(v->databox[0], yviewsize - v->databox[3], 0.f);
			glTexCoord2f(1.f, 1.f);               
			glVertex3f(v->databox[2], yviewsize - v->databox[3], 0.f);
			glTexCoord2f(1.f, 0.f);
			glVertex3f(v->databox[2], yviewsize - v->databox[1], 0.f);
			glTexCoord2f(0.f, 0.f);
			glVertex3f(v->databox[0], yviewsize - v->databox[1], 0.f);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		/* render databox line */
		glPushAttrib(GL_CURRENT_BIT);
		glEnable(GL_LINE_STIPPLE);
		glColor3f(0.3f, .3f, .3f);
		glLineStipple(1, 0x3333);
		glBegin(GL_LINE_LOOP);
			glVertex3f(v->databox[0], yviewsize - v->databox[3], 0.f);
			glVertex3f(v->databox[0], yviewsize - v->databox[1], 0.f);
			glVertex3f(v->databox[2], yviewsize - v->databox[1], 0.f);
			glVertex3f(v->databox[2], yviewsize - v->databox[3], 0.f);
		glEnd();
		glPopAttrib();
	}

	/* render viewbox line */
	glTranslatef(0.f, 0.f, 0.1f); 
	glPushAttrib(GL_CURRENT_BIT);
	glLineStipple(1, 0x0F0F);
	glColor3f(.5f, .5f, .5f);
	glBegin(GL_LINE_LOOP);
		glVertex3f(v->viewbox[0], v->viewbox[1], 0.f);
		glVertex3f(v->viewbox[0], v->viewbox[3], 0.f);
		glVertex3f(v->viewbox[2], v->viewbox[3], 0.f);
		glVertex3f(v->viewbox[2], v->viewbox[1], 0.f);
	glEnd();
	glPopAttrib();

	/* render tile guide */
	if (v->tilesize > 0 && v->draw_tile == 1) {
		glTranslatef(0.f, 0.f, 0.2f); 
		glPushAttrib(GL_CURRENT_BIT);
		glLineStipple(1, 0x3333);
		glColor3f(.5f, .5f, .5f);
		GlDrawTileGuide(xviewsize, yviewsize, v->tilesize);
		glPopAttrib();
	}

	/* Not swapping the buffers here is intentional. */
}

void FbvResize(struct FrameBufferViewer *v, int width, int height)
{
	v->win_width = width;
	v->win_height = height;

	glViewport(0, 0, v->win_width, v->win_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-.5f * v->win_width, .5f * v->win_width,
	        -.5f * v->win_height, .5f * v->win_height,
	        -1.f, 1.f);
}

void FbvPressButton(struct FrameBufferViewer *v, MouseButton button, int x, int y)
{
	v->xpresspos = x;
	v->ypresspos = y;

	switch (button) {
	case MB_LEFT:
#if 0
		{
			const float *buf;
			float pixel[4] = {0, 0, 0, 1};
			int bufx, bufy;
			int view_width;
			int y_inv;
			GLdouble wx;
			GLdouble wy;
			GLdouble wz;
			GLint view[4];
			GLdouble model[16];
			GLdouble proj[16];

			glGetIntegerv(GL_VIEWPORT, view);
			glGetDoublev(GL_MODELVIEW_MATRIX, model);
			glGetDoublev(GL_PROJECTION_MATRIX, proj);
			y_inv=view[3]-(GLint)y-1;
			printf ("Coordinates at cursor are (%4d, %4d)\n",x, y_inv);
			/*
			*/
			gluUnProject(x, y_inv, 0., model, proj, view, &wx, &wy, &wz);
			printf ("World coords at z=0.0 are (%g, %g, %g)\n", wx, wy, wz);
			/*
			gluUnProject(x, y_inv, 1., model, proj, view, &wx, &wy, &wz);
			printf ("World coords at z=1.0 are (%g, %g, %g)\n", wx, wy, wz);
			*/
			/*
			*/
			view_width = v->viewbox[2] - v->viewbox[0];
			bufx = (int) wx - v->viewbox[0];
			bufy = (int) wy - v->viewbox[1] * view_width;
			/*
			bufy = (v->databox[3]-v->databox[1]-1) - bufy;
			printf ("++++++(%d, %d)\n", bufx, (v->databox[3]-v->databox[1]-1) - bufy);
			*/
			bufy = FbGetHeight(v->fb) - bufy - 1;
			/*
			printf ("------(%d, %d)\n", bufx, bufy);
			bufx = CLAMP(bufx, 0, v->databox[2]-v->databox[0]-1);
			bufy = CLAMP(bufy, 0, v->databox[3]-v->databox[1]-1);
			*/
			bufx = CLAMP(bufx, 0, FbGetWidth(v->fb)-1);
			bufy = CLAMP(bufy, 0, FbGetHeight(v->fb)-1);
			/*
			printf ("(%d, %d)\n", bufx, bufy);
			*/
			buf = FbGetReadOnly(v->fb, (int) bufx, (int) bufy, 0);
			memcpy(pixel, buf, sizeof(float) * FbGetChannelCount(v->fb));
			printf("R:%g G:%g B:%g A:%g\n", pixel[0], pixel[1], pixel[2], pixel[3]);
        }
#endif
		break;
	case MB_MIDDLE:
		v->pressbutton = MB_MIDDLE;
		v->xlockoffset = v->xoffset;
		v->ylockoffset = v->yoffset;
		v->dist_per_pixel = 1.f/v->scale;
		break;
	case MB_RIGHT:
		v->pressbutton = MB_RIGHT;
		v->lockexponent = v->exponent;
		break;
	default:
		break;
	}
}

void FbvReleaseButton(struct FrameBufferViewer *v, MouseButton button, int x, int y)
{
	v->pressbutton = MB_NONE;
}

void FbvMoveMouse(struct FrameBufferViewer *v, int x, int y)
{
	const int posx = x;
	const int posy = y;

	switch (v->pressbutton) {
	case MB_MIDDLE:
		v->xoffset = v->xlockoffset + v->dist_per_pixel * (posx - v->xpresspos);
		v->yoffset = v->ylockoffset - v->dist_per_pixel * (posy - v->ypresspos);
		break;
	case MB_RIGHT:
		v->exponent = v->lockexponent + .01f * (float)(
					(posx - v->xpresspos) -
					(posy - v->ypresspos));
		v->exponent = CLAMP(v->exponent, -5.f, 10.f);
		v->scale = pow(2, v->exponent);
		break;
	default:
		break;
	}
}

void FbvPressKey(struct FrameBufferViewer *v, unsigned char key, int mouse_x, int mouse_y)
{
	switch (key) {
	case 'h':
		set_to_home_position(v);
		FbvDraw(v);
		break;
	case 'r':
		v->disp_chans = ( v->disp_chans == DISP_R ) ? DISP_RGB : DISP_R;
		initialize_gl(v);
		FbvDraw(v);
		break;
	case 'g':
		v->disp_chans = ( v->disp_chans == DISP_G ) ? DISP_RGB : DISP_G;
		initialize_gl(v);
		FbvDraw(v);
		break;
	case 'b':
		v->disp_chans = ( v->disp_chans == DISP_B ) ? DISP_RGB : DISP_B;
		initialize_gl(v);
		FbvDraw(v);
		break;
	case 'a':
		v->disp_chans = ( v->disp_chans == DISP_A ) ? DISP_RGB : DISP_A;
		initialize_gl(v);
		FbvDraw(v);
		break;
	case 't':
		v->draw_tile = (v->draw_tile == 1) ? 0 : 1;
		FbvDraw(v);
		break;
	case 'u':
		FbvLoadImage(v, v->filename);
		initialize_gl(v);
		FbvDraw(v);
		break;
	case '\033': /* ESC ASCII code */
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}
}

int FbvLoadImage(struct FrameBufferViewer *v, const char *filename)
{
	char try_filename[1024] = {'\0'};
	const size_t MAXCPY = 1024-1;
	const char *ext;
	int err;

	StrCopyAndTerminate(try_filename, filename, MAXCPY);
	if (strcmp(v->filename, try_filename) != 0) {
		StrCopyAndTerminate(v->filename, try_filename, MAXCPY);
	}

	ext = file_extension(v->filename);
	if (ext == NULL)
		return -1;

	if (strcmp(ext, "fb") == 0) {
		err = load_fb(v);
	}
	else if (strcmp(ext, "mip") == 0) {
		err = load_mip(v);
	}
	else {
		err = -1;
	}

	{
		/* TODO define gamma function */
		float *pixel = FbGetWritable(v->fb, 0, 0, 0);
		const int N = FbGetWidth(v->fb) * FbGetHeight(v->fb) * FbGetChannelCount(v->fb);
		int i;

		const float gamma = 1 / 2.2;

		for (i = 0; i < N; i++) {
			pixel[i] = pow(pixel[i], gamma);
		}
	}

	return err;
}

void FbvGetImageSize(const struct FrameBufferViewer *v,
		int databox[4], int viewbox[4], int *nchannels)
{
	BOX2_COPY(databox, v->databox);
	BOX2_COPY(viewbox, v->viewbox);
	*nchannels = FbGetChannelCount(v->fb);
}

/*----------------------------------------------------------------------------*/
static void initialize_gl(struct FrameBufferViewer *v)
{
	GLenum format;

	switch (FbGetChannelCount(v->fb)) {
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		assert(!"invalid channel count");
		break;
	}

	if (v->fb != NULL && !FbIsEmpty(v->fb)) {
		float m[16] = {
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f};

		glMatrixMode(GL_COLOR);
		switch (v->disp_chans) {
		case DISP_RGB:
			m[0] = m[5] = m[10] = 1.f;
			break;
		case DISP_R:
			m[0] = m[1] = m[2] = 1.f;
			break;
		case DISP_G:
			m[4] = m[5] = m[6] = 1.f;
			break;
		case DISP_B:
			m[8] = m[9] = m[10] = 1.f;
			break;
		case DISP_A:
			m[12] = m[13] = m[14] = 1.f;
			break;
		default:
			break;
		}
		glLoadMatrixf(m);

		glPixelStorei(GL_UNPACK_ALIGNMENT, FbGetChannelCount(v->fb));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FbGetWidth(v->fb), FbGetHeight(v->fb), 0,
					  format, GL_FLOAT, FbGetReadOnly(v->fb, 0, 0, 0));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}

static void set_to_home_position(struct FrameBufferViewer *v)
{
	v->scale = 1.f;
	v->exponent = 0.f;
	v->lockexponent = 0.f;
	v->dist_per_pixel = 0.f;
	v->xoffset = 0.f;
	v->yoffset = 0.f;
	v->xpresspos = 0;
	v->ypresspos = 0;
	v->xlockoffset = 0.f;
	v->ylockoffset = 0.f;
}

static int load_fb(struct FrameBufferViewer *v)
{
	struct FbInput *in;

	in = FbOpenInputFile(v->filename);
	if (in == NULL)
		return -1;

	if (FbReadHeader(in)) {
		FbCloseInputFile(in);
		return -1;
	}

	if (v->fb == NULL) {
		v->fb = FbNew();
	}

	if (v->fb == NULL) {
		FbCloseInputFile(in);
		return -1;
	}

	FbResize(v->fb, in->width, in->height, in->nchannels);
	BOX2_COPY(v->viewbox, in->viewbox);
	BOX2_COPY(v->databox, in->databox);
	in->data = FbGetWritable(v->fb, 0, 0, 0);

	FbReadData(in);
	FbCloseInputFile(in);

	v->tilesize = 0;

	return 0;
}

static int load_mip(struct FrameBufferViewer *v)
{
	struct MipInput *in;

	in = MipOpenInputFile(v->filename);
	if (in == NULL)
		return -1;

	if (MipReadHeader(in)) {
		MipCloseInputFile(in);
		return -1;
	}

	if (v->fb == NULL) {
		v->fb = FbNew();
	}

	if (v->fb == NULL) {
		MipCloseInputFile(in);
		return -1;
	}

	FbResize(v->fb, in->width, in->height, in->nchannels);

	BOX2_SET(v->viewbox, 0, 0, in->width, in->height);
	BOX2_COPY(v->databox, v->viewbox);

	{
		int x, y;
		struct FrameBuffer *tilebuf;

		tilebuf = FbNew();
		if (tilebuf == NULL) {
		}
		FbResize(tilebuf, in->tilesize, in->tilesize, in->nchannels);

		for (y = 0; y < in->yntiles; y++) {
			for (x = 0; x < in->xntiles; x++) {
				int i;
				in->data = FbGetWritable(tilebuf, 0, 0, 0);
				MipReadTile(in, x, y);
				for (i = 0; i < in->tilesize; i++) {
					float *dst;
					const float *src;
					dst = FbGetWritable(v->fb, x * in->tilesize, y * in->tilesize + i, 0);
					src = FbGetReadOnly(tilebuf, 0, i, 0);
					memcpy(dst, src, sizeof(float) * in->tilesize * in->nchannels);
				}
			}
		}
		FbFree(tilebuf);
	}

	v->tilesize = in->tilesize;
	MipCloseInputFile(in);

	return 0;
}

static const char *file_extension(const char *filename)
{
	const char *s = strrchr(filename, '.');
	if (s == NULL)
		return 0;

	return s + 1;
}

void GlDrawTileGuide(int width, int height, int tilesize)
{
	int i;
	const int XNLINES = width / tilesize + 1;
	const int YNLINES = height / tilesize + 1;

	glBegin(GL_LINES);
	for (i = 0; i < XNLINES; i++) {
		glVertex3f(tilesize * i,      0, 0);
		glVertex3f(tilesize * i, height, 0);
	}
	for (i = 0; i < YNLINES; i++) {
		glVertex3f(0,     tilesize * i, 0);
		glVertex3f(width, tilesize * i, 0);
	}
	glEnd();
}

