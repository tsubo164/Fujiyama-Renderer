// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FRAMEBUFFER_VIEWER_H
#define FRAMEBUFFER_VIEWER_H

namespace fj {

enum MouseButton {
  MOUSE_BUTTON_NONE = 0,
  MOUSE_BUTTON_LEFT,
  MOUSE_BUTTON_MIDDLE,
  MOUSE_BUTTON_RIGHT
};

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
