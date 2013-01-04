/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

struct FrameBuffer;

extern struct FrameBuffer *FbNew(void);
extern void FbFree(struct FrameBuffer *fb);

extern int FbGetWidth(const struct FrameBuffer *fb);
extern int FbGetHeight(const struct FrameBuffer *fb);
extern int FbGetChannelCount(const struct FrameBuffer *fb);

extern float *FbResize(struct FrameBuffer *fb, int width, int height, int nchannels);
extern int FbComputeBounds(struct FrameBuffer *fb, int *bounds);
extern int FbIsEmpty(const struct FrameBuffer *fb);

extern float *FbGetWritable(struct FrameBuffer *fb, int x, int y, int z);
extern const float *FbGetReadOnly(const struct FrameBuffer *fb, int x, int y, int z);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

