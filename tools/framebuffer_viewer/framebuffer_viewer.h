/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FRAMEBUFFERVIEWER_H
#define FRAMEBUFFERVIEWER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MouseButton {
  MB_NONE = 0,
  MB_LEFT,
  MB_MIDDLE,
  MB_RIGHT
} MouseButton;

struct FrameBufferViewer;

extern struct FrameBufferViewer *FbvNewViewer(void);
extern void FbvFreeViewer(struct FrameBufferViewer *v);

extern void FbvDraw(const struct FrameBufferViewer *v);
extern void FbvResize(struct FrameBufferViewer *v, int width, int height);
extern void FbvPressButton(struct FrameBufferViewer *v, MouseButton button, int x, int y);
extern void FbvReleaseButton(struct FrameBufferViewer *v, MouseButton button, int x, int y);
extern void FbvMoveMouse(struct FrameBufferViewer *v, int x, int y);
extern void FbvPressKey(struct FrameBufferViewer *v, unsigned char key, int mouse_x, int mouse_y);

extern int FbvLoadImage(struct FrameBufferViewer *v, const char *filename);
extern void FbvGetImageSize(const struct FrameBufferViewer *v,
    int databox[4], int viewbox[4], int *nchannels);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */
