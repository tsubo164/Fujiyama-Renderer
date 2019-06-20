// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FRAMEBUFFER_VIEWER_H
#define FRAMEBUFFER_VIEWER_H

#include "fj_compatibility.h"
#include "fj_framebuffer.h"
#include "fj_rectangle.h"
#include "fj_socket.h"
#include "image_card.h"

#include <string>
#include <vector>

namespace fj {

typedef void (*WindowResizeRequest)(void *window, int x_image_res, int y_image_res);
typedef void (*WindowChangeTitleRequest)(void *window, const char *title);

enum MouseButton {
  MOUSE_BUTTON_NONE = 0,
  MOUSE_BUTTON_LEFT,
  MOUSE_BUTTON_MIDDLE,
  MOUSE_BUTTON_RIGHT
};

class FrameBufferViewer {
public:
  FrameBufferViewer();
  ~FrameBufferViewer();

  void Draw() const;
  void Resize(int width, int height);

  void PressButton(MouseButton button, int x, int y);
  void ReleaseButton(MouseButton button, int x, int y);
  void MoveMouse(int x, int y);
  void PressKey(unsigned char key, int mouse_x, int mouse_y);

  void StartListening();
  void StopListening();
  bool IsListening() const;
  void Listen();
  bool IsRendering() const;

  int LoadImage(const std::string &filename);

  void GetImageSize(Rectangle &viewbox, int *nchannels) const;

  void SetWindowResizeRequest(
      void *window_object,
      WindowResizeRequest resize_window);
  void SetWindowChangeTitleRequest(
      void *window_object,
      WindowChangeTitleRequest change_window_title);

private:
  void set_to_home_position();
  void setup_image_card();
  void draw_viewbox() const;

  FrameBuffer fb_;
  ImageCard image_;

  std::string filename_;
  bool is_listening_;

  int win_width_;
  int win_height_;
  int diplay_channel_;

  MouseButton pressbutton_;
  int xpresspos_;
  int ypresspos_;

  float scale_;
  float exponent_;
  float lockexponent_;
  float dist_per_pixel_;
  int xoffset_;
  int yoffset_;
  int xlockoffset_;
  int ylockoffset_;

  //int viewbox_[4];
  Rectangle viewbox_;

  int tilesize_;
  int draw_tile_;

  //TODO make class for managing callbacks
  void *window_object_;
  WindowResizeRequest resize_window_;
  WindowChangeTitleRequest change_window_title_;

  //TODO make class for icp/status management
  Socket server_;
  int state_;
  enum {
    STATE_NONE = 0,
    STATE_READY,
    STATE_RENDERING,
    STATE_DONE,
    STATE_INTERRUPTED,
    STATE_ABORT
  };

  class TileStatus {
  public:
    TileStatus() :
        state(STATE_READY), region()
    {}
    ~TileStatus() {}

    int state;
    Rectangle region;
  };
  std::vector<TileStatus> tile_status_;
  int32_t frame_id_;
};

} // namespace xxx

#endif // XXX_H
