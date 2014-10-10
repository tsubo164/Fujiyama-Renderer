// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "framebuffer_viewer.h"
#include "fj_framebuffer_io.h"
#include "fj_rectangle.h"
#include "fj_numeric.h"
#include "fj_mipmap.h"
#include "fj_box.h"

#include "fj_color.h"

#include "compatible_opengl.h"
#include "load_images.h"

#include <iostream>
#include <cmath>
#include <cassert>

namespace fj {

static void draw_tile_guide(int width, int height, int tilesize);

FrameBufferViewer::FrameBufferViewer() :
    is_listening_(false),
    win_width_(0),
    win_height_(0),
    diplay_channel_(DISPLAY_RGB),
    pressbutton_(MOUSE_BUTTON_NONE),
    tilesize_(0),
    draw_tile_(1)
{
  set_to_home_position();
  BOX2_SET(databox_, 0, 0, 0, 0);
  BOX2_SET(viewbox_, 0, 0, 0, 0);

  //TODO TEST
  //StartListening();
#if 0
  server_.Bind();
  server_.Listen();
#endif
}

FrameBufferViewer::~FrameBufferViewer()
{
}

void FrameBufferViewer::Draw() const
{
  const int xviewsize = viewbox_[2] - viewbox_[0];
  const int yviewsize = viewbox_[3] - viewbox_[1];
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
    glVertex3f(viewbox_[0], viewbox_[1], 0.f);
    glVertex3f(viewbox_[2], viewbox_[1], 0.f);
    glVertex3f(viewbox_[2], viewbox_[3], 0.f);
    glVertex3f(viewbox_[0], viewbox_[3], 0.f);
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
      gluUnProject(x, y_inv, 0., model, proj, view, &wx, &wy, &wz);
      printf ("World coords at z=0.0 are (%g, %g, %g)\n", wx, wy, wz);
      //gluUnProject(x, y_inv, 1., model, proj, view, &wx, &wy, &wz);
      //printf ("World coords at z=1.0 are (%g, %g, %g)\n", wx, wy, wz);
      view_width = viewbox[2] - viewbox[0];
      bufx = (int) wx - viewbox[0];
      bufy = (int) wy - viewbox[1] * view_width;
      //bufy = (databox[3]-databox[1]-1) - bufy;
      //printf ("++++++(%d, %d)\n", bufx, (databox[3]-databox[1]-1) - bufy);
      bufy = FbGetHeight(fb) - bufy - 1;
      //printf ("------(%d, %d)\n", bufx, bufy);
      //bufx = CLAMP(bufx, 0, databox[2]-databox[0]-1);
      //bufy = CLAMP(bufy, 0, databox[3]-databox[1]-1);
      bufx = CLAMP(bufx, 0, FbGetWidth(fb)-1);
      bufy = CLAMP(bufy, 0, FbGetHeight(fb)-1);
      //printf ("(%d, %d)\n", bufx, bufy);
      buf = FbGetReadOnly(fb, (int) bufx, (int) bufy, 0);
      memcpy(pixel, buf, sizeof(float) * FbGetChannelCount(fb));
      printf("R:%g G:%g B:%g A:%g\n", pixel[0], pixel[1], pixel[2], pixel[3]);
        }
#endif
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
    is_listening_ = !is_listening_;
#if 0
    {
      FrameBuffer tmp;
      tmp.Resize(2, 2, 4);
      for (int y = 0; y < tmp.GetHeight(); y++) {
        for (int x = 0; x < tmp.GetWidth(); x++) {
          const Color4 color((x + y)%3 == 0, (x + y)%3 == 1, (x + y)%3 == 2, 0);
          tmp.SetColor(x, y, color);
        }
      }

      Copy(fb_, tmp, -1, 1);
    }
    setup_image_card();
    Draw();
#endif
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
  case '\033': // ESC ASCII code
    exit(EXIT_SUCCESS);
    break;
  default:
    break;
  }
}

void FrameBufferViewer::StartListening()
{
  if (IsListening())
    return;

  server_.Open();
  server_.Bind();
  server_.Listen();
  is_listening_ = true;
}

void FrameBufferViewer::StopListening()
{
  if (!IsListening())
    return;

  server_.Close();
  is_listening_ = false;
}

bool FrameBufferViewer::IsListening() const
{
  return is_listening_;
}

void FrameBufferViewer::Listen()
{
  //Socket server;
  Socket client;

  const int timeout_sec = 0;
  const int timeout_micro_sec = 0 * 100 * 1000;
  const int result = server_.AcceptOrTimeout(client, timeout_sec, timeout_micro_sec);

  if (result == -1) {
    std::cout << "error\n";
    // TODO ERROR HANDLING
  }
  else if (result == 0) {
    // time out
  }
  else {
    std::cout << "server: accepted\n";

    int32_t size = 0;
    client.Receive(reinterpret_cast<char *>(&size), sizeof(size));
    std::cout << "size: " << size << "\n";

    int32_t type = 0;
    client.Receive(reinterpret_cast<char *>(&type), sizeof(type));
    std::cout << "type: " << type << "\n";

    int32_t id = 0;
    client.Receive(reinterpret_cast<char *>(&id), sizeof(id));
    std::cout << "id: " << id << "\n";

    int32_t msg[3];
    msg[0] = 2 * sizeof(msg[0]);
    msg[1] = 9999;
    msg[2] = id;

    client.Send(reinterpret_cast<char *>(msg), 3 * sizeof(msg[0]));
#if 0
    char ch[5];
    client.Receive(ch, 5);
    std::cout << "ch: " << ch[0] << "\n";

    int *j = reinterpret_cast<int *>(&ch[1]);
    //client.Receive(reinterpret_cast<char *>(j), sizeof(j));
    std::cout << "j: " << *j << "\n";
    const int i = ntohl(*j);
    std::cout << "i: " << i << "\n";
#endif
#if 0
    char ch;
    client.Receive(&ch, 1);
    std::cout << "ch: " << ch << "\n";

    int j = 0;
    client.Receive(reinterpret_cast<char *>(&j), sizeof(j));
    std::cout << "j: " << j << "\n";
    const int i = ntohl(j);
    std::cout << "i: " << i << "\n";
#endif
  }
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
    BOX2_COPY(viewbox_, info.viewbox);
    BOX2_COPY(databox_, info.databox);
    tilesize_ = info.tilesize;
  }
  else if (ext == "mip") {
    BufferInfo info;
    err = LoadMip(filename_, &fb_, &info);
    BOX2_COPY(viewbox_, info.viewbox);
    BOX2_COPY(databox_, info.databox);
    tilesize_ = info.tilesize;
  }
  else {
    return -1;
  }
  if (err) {
    return -1;
  }

  {
    // TODO define gamma function
    float *pixel = fb_.GetWritable(0, 0, 0);
    const int N = fb_.GetWidth() * fb_.GetHeight() * fb_.GetChannelCount();
    int i;

    const float gamma = 1 / 2.2;

    for (i = 0; i < N; i++) {
      pixel[i] = pow(pixel[i], gamma);
    }
  }

  setup_image_card();

  return err;
}

void FrameBufferViewer::GetImageSize(int databox[4], int viewbox[4], int *nchannels) const
{
  BOX2_COPY(databox, databox_);
  BOX2_COPY(viewbox, viewbox_);
  *nchannels = fb_.GetChannelCount();
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
  image_.Init(fb_.GetReadOnly(0, 0, 0),
      fb_.GetChannelCount(), diplay_channel_,
      databox_[0],
      viewbox_[3] - viewbox_[1] - databox_[3],
      databox_[2] - databox_[0],
      databox_[3] - databox_[1]);
}

void FrameBufferViewer::draw_viewbox() const
{
  float r, g, b;

  if (IsListening()) {
    r = .4f;
    g = .6f;
    b = .8f;
  } else {
    r = g = b = .5f;
  }

  glPushAttrib(GL_CURRENT_BIT);
  glLineStipple(1, 0x0F0F);
  glColor3f(r, g, b);
  glBegin(GL_LINE_LOOP);
    glVertex3f(viewbox_[0], viewbox_[1], 0.f);
    glVertex3f(viewbox_[0], viewbox_[3], 0.f);
    glVertex3f(viewbox_[2], viewbox_[3], 0.f);
    glVertex3f(viewbox_[2], viewbox_[1], 0.f);
  glEnd();
  glPopAttrib();
}

static void draw_tile_guide(int width, int height, int tilesize)
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

} // namespace xxx
