/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_FRAMEBUFFER_H
#define FJ_FRAMEBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

struct FrameBuffer;
struct Color4;

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

extern void FbGetColor(const struct FrameBuffer *fb, int x, int y, struct Color4 *rgba);
extern void FbSetColor(struct FrameBuffer *fb, int x, int y, const struct Color4 *rgba);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
