// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "framebuffer_viewer.h"
#include "fj_framebuffer_io.h"
#include "fj_rectangle.h"
#include "fj_protocol.h"
#include "fj_numeric.h"
#include "fj_mipmap.h"
#include "fj_color.h"
#include "fj_box.h"

#include "compatible_opengl.h"
#include "load_images.h"

#include <iostream>
#include <cmath>
#include <cassert>

namespace fj {

static bool is_socket_ready = false;
static void draw_tile_guide(int width, int height, int tilesize);

FrameBufferViewer::FrameBufferViewer() :
    is_listening_(false),
    win_width_(0),
    win_height_(0),
    diplay_channel_(DISPLAY_RGB),
    pressbutton_(MOUSE_BUTTON_NONE),

    viewbox_(),

    tilesize_(0),
    draw_tile_(1),
    window_object_(NULL),
    resize_window_(NULL),
    change_window_title_(NULL),
    server_(),
    state_(STATE_NONE),
    tile_status_(),
    frame_id_(-1)
{
  set_to_home_position();
}

FrameBufferViewer::~FrameBufferViewer()
{
  if (is_socket_ready) {
    const int err = SocketCleanup();
    if (err) {
      std::cerr << "SocketCleanup() failed: " << SocketErrorMessage() << "\n\n";
    }
    is_socket_ready = false;
  }
}

void FrameBufferViewer::Draw() const
{
  const int xviewsize = viewbox_.Size()[0];
  const int yviewsize = viewbox_.Size()[1];
  const int xmove = scale_ * xoffset_; 
  const int ymove = scale_ * yoffset_; 

  glClearColor(.2f, .2f, .2f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);

  if (fb_.IsEmpty()) {
    return;
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(xmove, ymove, 0.f);
  glScalef(scale_, scale_, 1.f);
  glTranslatef(-.5 * xviewsize, -.5 * yviewsize, 0.f); 

  // render background
  glColor3f(0.f, 0.f, 0.f);
  glBegin(GL_QUADS);
    glVertex3f(viewbox_.min[0], viewbox_.min[1], 0.f);
    glVertex3f(viewbox_.max[0], viewbox_.min[1], 0.f);
    glVertex3f(viewbox_.max[0], viewbox_.max[1], 0.f);
    glVertex3f(viewbox_.min[0], viewbox_.max[1], 0.f);
  glEnd();

  // render framebuffer
  glTranslatef(0.f, 0.f, 0.1f); 

  if (!fb_.IsEmpty()) {
    image_.Draw();
    image_.DrawOutline();
  }

  // render viewbox line
  glTranslatef(0.f, 0.f, 0.1f); 
  draw_viewbox();

  // render tile guide
  if (tilesize_ > 0 && draw_tile_ == 1) {
    glTranslatef(0.f, 0.f, 0.2f); 
    glPushAttrib(GL_CURRENT_BIT);
    glLineStipple(1, 0x3333);
    glColor3f(.5f, .5f, .5f);
    draw_tile_guide(xviewsize, yviewsize, tilesize_);
    glPopAttrib();
  }

  // render rendering guide
      glTranslatef(0.f, 0.f, 0.1f); 
      glPushAttrib(GL_CURRENT_BIT);
      glDisable(GL_LINE_STIPPLE);
      //glLineStipple(1, 0x3333);
      glColor3f(1, 1, 1);

      for (size_t i = 0; i < tile_status_.size(); i++)
      {
        if (tile_status_[i].state == STATE_RENDERING) {
          GLfloat xmin = tile_status_[i].region.min[0] + 5 * 0;
          GLfloat ymin = tile_status_[i].region.min[1] + 5 * 0;
          GLfloat xmax = tile_status_[i].region.max[0] - 5 * 0;
          GLfloat ymax = tile_status_[i].region.max[1] - 5 * 0;
          ymin = yviewsize - ymin;
          ymax = yviewsize - ymax;
          glBegin(GL_LINE_LOOP);
            glVertex3f(xmin, ymin, 0.f);
            glVertex3f(xmin, ymax, 0.f);
            glVertex3f(xmax, ymax, 0.f);
            glVertex3f(xmax, ymin, 0.f);
          glEnd();
        }
      }
      glPopAttrib();

  // Not swapping the buffers here is intentional.
}

void FrameBufferViewer::Resize(int width, int height)
{
  win_width_ = width;
  win_height_ = height;

  glViewport(0, 0, win_width_, win_height_);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-.5f * win_width_, .5f * win_width_,
          -.5f * win_height_, .5f * win_height_,
          -1.f, 1.f);
}

void FrameBufferViewer::PressButton(MouseButton button, int x, int y)
{
  xpresspos_ = x;
  ypresspos_ = y;

  switch (button) {
  case MOUSE_BUTTON_LEFT:
    break;
  case MOUSE_BUTTON_MIDDLE:
    pressbutton_ = MOUSE_BUTTON_MIDDLE;
    xlockoffset_ = xoffset_;
    ylockoffset_ = yoffset_;
    dist_per_pixel_ = 1.f/scale_;
    break;
  case MOUSE_BUTTON_RIGHT:
    pressbutton_ = MOUSE_BUTTON_RIGHT;
    lockexponent_ = exponent_;
    break;
  default:
    break;
  }
}

void FrameBufferViewer::ReleaseButton(MouseButton button, int x, int y)
{
  pressbutton_ = MOUSE_BUTTON_NONE;
}

void FrameBufferViewer::MoveMouse(int x, int y)
{
  const int posx = x;
  const int posy = y;

  switch (pressbutton_) {
  case MOUSE_BUTTON_MIDDLE:
    xoffset_ = xlockoffset_ + dist_per_pixel_ * (posx - xpresspos_);
    yoffset_ = ylockoffset_ - dist_per_pixel_ * (posy - ypresspos_);
    break;
  case MOUSE_BUTTON_RIGHT:
    exponent_ = lockexponent_ + .01f * (float)(
          (posx - xpresspos_) -
          (posy - ypresspos_));
    exponent_ = Clamp(exponent_, -5.f, 10.f);
    scale_ = pow(2, exponent_);
    break;
  default:
    break;
  }
}

void FrameBufferViewer::PressKey(unsigned char key, int mouse_x, int mouse_y)
{
  switch (key) {
  // TODO TEST
  case 'l':
    if (IsListening()) {
      StopListening();
    } else {
      StartListening();
    }
    break;
  case 'h':
    set_to_home_position();
    Draw();
    break;
  case 'r':
    diplay_channel_ = (diplay_channel_ == DISPLAY_R) ? DISPLAY_RGB : DISPLAY_R;
    setup_image_card();
    Draw();
    break;
  case 'g':
    diplay_channel_ = (diplay_channel_ == DISPLAY_G) ? DISPLAY_RGB : DISPLAY_G;
    setup_image_card();
    Draw();
    break;
  case 'b':
    diplay_channel_ = (diplay_channel_ == DISPLAY_B) ? DISPLAY_RGB : DISPLAY_B;
    setup_image_card();
    Draw();
    break;
  case 'a':
    diplay_channel_ = (diplay_channel_ == DISPLAY_A) ? DISPLAY_RGB : DISPLAY_A;
    setup_image_card();
    Draw();
    break;
  case 't':
    draw_tile_ = (draw_tile_ == 1) ? 0 : 1;
    Draw();
    break;
  case 'u':
    LoadImage(filename_);
    setup_image_card();
    Draw();
    break;
  case 'q':
    exit(EXIT_SUCCESS);
    break;
  case '\033': // ESC ASCII code
    if (state_ == STATE_RENDERING) {
      state_ = STATE_INTERRUPTED;
    }
    break;
  default:
    break;
  }
}

void FrameBufferViewer::StartListening()
{
  if (IsListening())
    return;

  if (!is_socket_ready) {
    const int err = SocketStartup();
    if (err) {
      std::cerr << "SocketStartup() failed: " << SocketErrorMessage() << "\n\n";
      return;
    } else {
      is_socket_ready = true;
    }
  }

  server_.Open();
  server_.EnableReuseAddr();
  server_.Bind();
  server_.Listen();
  is_listening_ = true;

  state_ = STATE_READY;
  frame_id_ = -1;

  if (change_window_title_ != NULL) {
    std::string title("READY: listeing to renderer"); 
    change_window_title_(window_object_, title.c_str());
  }
}

void FrameBufferViewer::StopListening()
{
  if (!IsListening())
    return;

  server_.Shutdown();
  server_.Close();
  is_listening_ = false;

  state_ = STATE_NONE;
  frame_id_ = -1;

  if (change_window_title_ != NULL) {
    std::string title("DISCONNECTED: 'l' to start listeing"); 
    change_window_title_(window_object_, title.c_str());
  }
}

bool FrameBufferViewer::IsListening() const
{
  return is_listening_;
}

void FrameBufferViewer::Listen()
{
  Socket client;

  const int timeout_sec = 0;
  const int timeout_micro_sec = 0;
  const int result = server_.AcceptOrTimeout(client, timeout_sec, timeout_micro_sec);

  if (result == -1) {
    // TODO ERROR HANDLING
    return;
  }
  else if (result == 0) {
    // time out
    return;
  }
  else {
    Message message;
    FrameBuffer tilebuf;

    int e = ReceiveMessage(client, message, tilebuf);
    if (e < 0) {
      // TODO ERROR HANDLING
    }
    client.ShutdownRead();

    switch (message.type) {

    case MSG_RENDER_FRAME_START:
      if (frame_id_ > 0) {
        // TODO ERROR HANDLING
        std::cerr << "WARNING: fbview recieved another frame ID: " << message.frame_id << "\n";
        std::cerr << "WARNING: fbview ignored this message.\n\n";
        return;
      }
      frame_id_   = message.frame_id;
      fb_.Resize(message.xres, message.yres, message.channel_count);
      viewbox_.min[0] = 0;
      viewbox_.min[1] = 0;
      viewbox_.max[0] = message.xres;
      viewbox_.max[1] = message.yres;
      setup_image_card();
      tile_status_.clear();
      tile_status_.resize(message.tile_count);
      state_ = STATE_RENDERING;

      if (resize_window_ != NULL) {
        const int new_window_size_x = viewbox_.Size()[0];
        const int new_window_size_y = viewbox_.Size()[1];
        resize_window_(window_object_, new_window_size_x, new_window_size_y);
      }
      if (change_window_title_ != NULL) {
        std::string title("RENDERING: ESC to stop rendering"); 
        change_window_title_(window_object_, title.c_str());
      }
      break;

    case MSG_RENDER_FRAME_DONE:
      if (frame_id_ != message.frame_id) {
        std::cerr << "WARNING: fbview recieved another frame ID: " << message.frame_id << "\n";
        std::cerr << "WARNING: fbview ignored this message.\n";
        return;
      }
      if (state_ == STATE_RENDERING) {
        state_ = STATE_READY;
        if (change_window_title_ != NULL) {
          std::string title("READY: listeing to renderer"); 
          change_window_title_(window_object_, title.c_str());
        }
      } else if (state_ == STATE_INTERRUPTED) {
        for (size_t i = 0; i < tile_status_.size(); i++) {
          tile_status_[i] = TileStatus();
        }
        state_ = STATE_ABORT;
        if (change_window_title_ != NULL) {
          std::string title("INCOMPLETE: rendering terminated"); 
          change_window_title_(window_object_, title.c_str());
        }
      }
      frame_id_ = -1;

      break;

    case MSG_RENDER_TILE_START:
      if (frame_id_ != message.frame_id) {
        return;
      }
      tile_status_[message.tile_id].region.min[0] = message.xmin;
      tile_status_[message.tile_id].region.min[1] = message.ymin;
      tile_status_[message.tile_id].region.max[0] = message.xmax;
      tile_status_[message.tile_id].region.max[1] = message.ymax;
      tile_status_[message.tile_id].state = STATE_RENDERING;
      break;

    case MSG_RENDER_TILE_DONE:
      if (frame_id_ != message.frame_id) {
        return;
      }
      tile_status_[message.tile_id].region.min[0] = message.xmin;
      tile_status_[message.tile_id].region.min[1] = message.ymin;
      tile_status_[message.tile_id].region.max[0] = message.xmax;
      tile_status_[message.tile_id].region.max[1] = message.ymax;
      tile_status_[message.tile_id].state = STATE_DONE;

      // Gamma
      for (int y = 0; y < tilebuf.GetHeight(); y++) {
        for (int x = 0; x < tilebuf.GetWidth(); x++) {
          const Color4 color = tilebuf.GetColor(x, y);
          tilebuf.SetColor(x, y, Gamma(color, 1/2.2));
        }
      }

      PasteInto(fb_, tilebuf,
          tile_status_[message.tile_id].region.min[0],
          tile_status_[message.tile_id].region.min[1]);

      setup_image_card();
      break;

    default:
      // TODO ERROR HANDLING
      break;
    }

    if (state_ == STATE_INTERRUPTED) {
      if (change_window_title_ != NULL) {
        std::string title("INTERRUPTED: aborting render process"); 
        change_window_title_(window_object_, title.c_str());
      }
      SendRenderFrameAbort(client, message.frame_id);
      // abort;
    }
  }
}

bool FrameBufferViewer::IsRendering() const
{
  return state_ == STATE_RENDERING;
}

int FrameBufferViewer::LoadImage(const std::string &filename)
{
  int err = 0;

  if (filename_ != filename) {
    filename_ = filename;
  }

  const std::string ext = GetFileExtension(filename_);
  if (ext == "") {
    return -1;
  }

  if (ext == "fb") {
    BufferInfo info;
    err = LoadFb(filename_, &fb_, &info);
    viewbox_ = info.viewbox;
    tilesize_ = info.tilesize;
  }
  else if (ext == "mip") {
    BufferInfo info;
    err = LoadMip(filename_, &fb_, &info);
    viewbox_ = info.viewbox;
    tilesize_ = info.tilesize;
  }
  else {
    return -1;
  }
  if (err) {
    return -1;
  }

  // Gamma
  for (int y = 0; y < fb_.GetHeight(); y++) {
    for (int x = 0; x < fb_.GetWidth(); x++) {
      const Color4 color = fb_.GetColor(x, y);
      fb_.SetColor(x, y, Gamma(color, 1/2.2));
    }
  }

  setup_image_card();

  if (resize_window_ != NULL) {
    resize_window_(
        window_object_,
        viewbox_.Size()[0],
        viewbox_.Size()[1]);
  }
  if (change_window_title_ != NULL) {
    change_window_title_(window_object_, filename_.c_str());
  }

  return err;
}

void FrameBufferViewer::GetImageSize(Rectangle &viewbox, int *nchannels) const
{
  viewbox = viewbox_;
  *nchannels = fb_.GetChannelCount();
}

void FrameBufferViewer::SetWindowResizeRequest(
    void *window_object,
    WindowResizeRequest resize_window)
{
  window_object_ = window_object;
  resize_window_ = resize_window;
}

void FrameBufferViewer::SetWindowChangeTitleRequest(
    void *window_object,
    WindowChangeTitleRequest change_window_title)
{
  window_object_ = window_object;
  change_window_title_ = change_window_title;
}

void FrameBufferViewer::set_to_home_position()
{
  scale_ = 1.f;
  exponent_ = 0.f;
  lockexponent_ = 0.f;
  dist_per_pixel_ = 0.f;
  xoffset_ = 0.f;
  yoffset_ = 0.f;
  xpresspos_ = 0;
  ypresspos_ = 0;
  xlockoffset_ = 0.f;
  ylockoffset_ = 0.f;
}

void FrameBufferViewer::setup_image_card()
{
  if (fb_.IsEmpty()) {
    return;
  }

  image_.Init(fb_.GetReadOnly(0, 0, 0),
      fb_.GetChannelCount(), diplay_channel_,
      viewbox_.min[0],
      -viewbox_.min[1],
      viewbox_.Size()[0],
      viewbox_.Size()[1]);
}

void FrameBufferViewer::draw_viewbox() const
{
  float r, g, b;

  switch (state_) {
  case STATE_NONE:
    r = g = b = .5f;
    break;
  case STATE_READY:
    r = .4f;
    g = .6f;
    b = 1.f;
    break;
  case STATE_RENDERING:
    r = .4f;
    g = 1.f;
    b = .6f;
    break;
  case STATE_INTERRUPTED:
    r = 1.f;
    g = 1.f;
    b = .0f;
    break;
  case STATE_ABORT:
    r = 1.f;
    g = .0f;
    b = .0f;
    break;
  default:
    r = g = b = .5f;
    break;
  }

  glPushAttrib(GL_CURRENT_BIT);
  glLineStipple(1, 0x0F0F);
  glColor3f(r, g, b);
  glBegin(GL_LINE_LOOP);
    glVertex3f(viewbox_.min[0], viewbox_.min[1], 0.f);
    glVertex3f(viewbox_.min[0], viewbox_.max[1], 0.f);
    glVertex3f(viewbox_.max[0], viewbox_.max[1], 0.f);
    glVertex3f(viewbox_.max[0], viewbox_.min[1], 0.f);
  glEnd();
  glPopAttrib();
}

static void draw_tile_guide(int width, int height, int tilesize)
{
  const int XNLINES = width  / tilesize + 1;
  const int YNLINES = height / tilesize + 1;

  glBegin(GL_LINES);
  for (int i = 0; i < XNLINES; i++) {
    glVertex3f(tilesize * i,      0, 0);
    glVertex3f(tilesize * i, height, 0);
  }
  for (int i = 0; i < YNLINES; i++) {
    glVertex3f(0,     tilesize * i, 0);
    glVertex3f(width, tilesize * i, 0);
  }
  glEnd();
}

} // namespace xxx
