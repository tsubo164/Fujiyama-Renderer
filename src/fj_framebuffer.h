/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_FRAMEBUFFER_H
#define FJ_FRAMEBUFFER_H

/* TODO delete BOX2 */
/* BOX2 */
/* BOX2[4] {min{0, 0}, max{0, 0}} */
#define BOX2_XSIZE(box) ((box)[2]-(box)[0])
#define BOX2_YSIZE(box) ((box)[3]-(box)[1])

#define BOX2_SET(dst,xmin,ymin,xmax,ymax) do { \
  (dst)[0] = (xmin); \
  (dst)[1] = (ymin); \
  (dst)[2] = (xmax); \
  (dst)[3] = (ymax); \
  } while(0)

#define BOX2_COPY(dst,a) do { \
  (dst)[0] = (a)[0]; \
  (dst)[1] = (a)[1]; \
  (dst)[2] = (a)[2]; \
  (dst)[3] = (a)[3]; \
  } while(0)

namespace fj {

struct FrameBuffer;
struct Color4;

extern struct FrameBuffer *FbNew(void);
extern void FbFree(struct FrameBuffer *fb);

extern int FbGetWidth(const struct FrameBuffer *fb);
extern int FbGetHeight(const struct FrameBuffer *fb);
extern int FbGetChannelCount(const struct FrameBuffer *fb);

/* Returns the head of buffer if resize is done, otherwise NULL. */
extern float *FbResize(struct FrameBuffer *fb, int width, int height, int nchannels);
/*
 * Returns 0 if fb is has rgba channels otherwise -1.
 * Bounds can be computed only when fb is an rgba buffer.
 */
extern int FbComputeBounds(struct FrameBuffer *fb, int *bounds);
extern int FbIsEmpty(const struct FrameBuffer *fb);

/* Use these functions with caution. */
extern float *FbGetWritable(struct FrameBuffer *fb, int x, int y, int z);
extern const float *FbGetReadOnly(const struct FrameBuffer *fb, int x, int y, int z);

/*
 * Get color at pixel (x, y).
 * (r, r, r, 1) will be returned when framebuffer is grayscale
 * (r, g, b, 1) will be returned when framebuffer is rgb
 * (r, g, b, a) will be returned when framebuffer is rgba
 */
extern void FbGetColor(const struct FrameBuffer *fb, int x, int y, struct Color4 *rgba);
/*
 * Set color at pixel (x, y).
 * (r)          will be set when framebuffer is grayscale
 * (r, g, b)    will be set when framebuffer is rgb
 * (r, g, b, a) will be set when framebuffer is rgba
 */
extern void FbSetColor(struct FrameBuffer *fb, int x, int y, const struct Color4 *rgba);

} // namespace fj

#endif /* FJ_XXX_H */
