/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FRAMEBUFFERIO_H
#define FRAMEBUFFERIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

struct FrameBuffer;

struct FbInput {
	FILE *file;
	int version;
	int width;
	int height;
	int nchannels;

	int viewbox[4];
	int databox[4];

	float *data;
};

struct FbOutput {
	FILE *file;
	int version;
	int width;
	int height;
	int nchannels;

	int viewbox[4];
	int databox[4];

	const float *data;
};

enum FbErrorNo {
	ERR_FB_NOERR = 0,
	ERR_FB_NOMEM,
	ERR_FB_NOFILE,
	ERR_FB_NOTFB,
	ERR_FB_BADVER
};

extern int FbGetErrorNo(void);

extern const char *FbGetErrorMessage(int err);

extern struct FbInput *FbOpenInputFile(const char *filename);

extern void FbCloseInputFile(struct FbInput *in);

extern int FbReadHeader(struct FbInput *in);

extern int FbReadData(struct FbInput *in);

extern struct FbOutput *FbOpenOutputFile(const char *filename);

extern void FbCloseOutputFile(struct FbOutput *out);

extern void FbWriteFile(struct FbOutput *out);

/* high level interface for saving framebuffer file */
extern int FbSaveCroppedData(struct FrameBuffer *fb, const char *filename);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

