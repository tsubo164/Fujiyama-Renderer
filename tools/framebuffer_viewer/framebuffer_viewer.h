// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FRAMEBUFFERVIEWER_H
#define FRAMEBUFFERVIEWER_H

namespace fj {

typedef enum MouseButton {
  MB_NONE = 0,
  MB_LEFT,
  MB_MIDDLE,
  MB_RIGHT
} MouseButton;

class FrameBufferViewer;

extern FrameBufferViewer *FbvNewViewer(void);
extern void FbvFreeViewer(FrameBufferViewer *v);

extern void FbvDraw(const FrameBufferViewer *v);
extern void FbvResize(FrameBufferViewer *v, int width, int height);
extern void FbvPressButton(FrameBufferViewer *v, MouseButton button, int x, int y);
extern void FbvReleaseButton(FrameBufferViewer *v, MouseButton button, int x, int y);
extern void FbvMoveMouse(FrameBufferViewer *v, int x, int y);
extern void FbvPressKey(FrameBufferViewer *v, unsigned char key, int mouse_x, int mouse_y);

extern int FbvLoadImage(FrameBufferViewer *v, const char *filename);
extern void FbvGetImageSize(const FrameBufferViewer *v,
    int databox[4], int viewbox[4], int *nchannels);

} // namespace xxx

#endif // FJ_XXX_H
