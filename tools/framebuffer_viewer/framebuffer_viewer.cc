// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "framebuffer_viewer.h"
#include "fj_string_function.h"
#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_rectangle.h"
#include "fj_numeric.h"
#include "fj_mipmap.h"
#include "fj_box.h"

#include "compatible_opengl.h"
#include "glsl_shaders.h"
#include "load_images.h"
#include "draw_image.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cassert>

namespace fj {

class FrameBufferViewer {
public:
  FrameBufferViewer() {}
  ~FrameBufferViewer() {}

public:
  FrameBuffer *fb;
  char filename[1024];

  int win_width;
  int win_height;
  int diplay_channel;

  MouseButton pressbutton;
  int xpresspos;
  int ypresspos;

  float scale;
  float exponent;
  float lockexponent;
  float dist_per_pixel;
  int xoffset;
  int yoffset;
  int xlockoffset;
  int ylockoffset;

  int databox[4];
  int viewbox[4];

  int tilesize;
  int draw_tile;

  ImageCard image;
};

static void clear_image_viewer(FrameBufferViewer *v);
static void setup_image_drawing(FrameBufferViewer *v);
static void set_to_home_position(FrameBufferViewer *v);
void GlDrawTileGuide(int width, int height, int tilesize);

static void clear_image_viewer(FrameBufferViewer *v)
{
  v->fb = NULL;

  v->win_width = 0;
  v->win_height = 0;
  v->diplay_channel = DISPLAY_RGB;

  v->pressbutton = MB_NONE;

  set_to_home_position(v);

  BOX2_SET(v->databox, 0, 0, 0, 0);
  BOX2_SET(v->viewbox, 0, 0, 0, 0);
  v->tilesize = 0;
  v->draw_tile = 1;

  init_image_drawer(&v->image);
}

FrameBufferViewer *FbvNewViewer(void)
{
  FrameBufferViewer *v = new FrameBufferViewer();
  
  clear_image_viewer(v);
  return v;
}

void FbvFreeViewer(FrameBufferViewer *v)
{
  if (v == NULL)
    return;

  if (v->fb != NULL)
    FbFree(v->fb);

  delete v;
}

void FbvDraw(const FrameBufferViewer *v)
{
  int xmove = 0;
  int ymove = 0;
  int xviewsize = v->viewbox[2] - v->viewbox[0];
  int yviewsize = v->viewbox[3] - v->viewbox[1];

  glClearColor(.2f, .2f, .2f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);

  if (v->fb == NULL)
    return;

  xmove = v->scale * v->xoffset; 
  ymove = v->scale * v->yoffset; 

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(xmove, ymove, 0.f);
  glScalef(v->scale, v->scale, 1.f);
  glTranslatef(-.5 * xviewsize, -.5 * yviewsize, 0.f); 

  // render background
  glColor3f(0.f, 0.f, 0.f);
  glBegin(GL_QUADS);
    glVertex3f(v->viewbox[0], v->viewbox[1], 0.f);
    glVertex3f(v->viewbox[2], v->viewbox[1], 0.f);
    glVertex3f(v->viewbox[2], v->viewbox[3], 0.f);
    glVertex3f(v->viewbox[0], v->viewbox[3], 0.f);
  glEnd();

  // render framebuffer
  glTranslatef(0.f, 0.f, 0.1f); 

  if (!v->fb->IsEmpty()) {
    draw_image(&v->image);
    draw_outline(&v->image);
  }

  // render viewbox line
  glTranslatef(0.f, 0.f, 0.1f); 
  glPushAttrib(GL_CURRENT_BIT);
  glLineStipple(1, 0x0F0F);
  glColor3f(.5f, .5f, .5f);
  glBegin(GL_LINE_LOOP);
    glVertex3f(v->viewbox[0], v->viewbox[1], 0.f);
    glVertex3f(v->viewbox[0], v->viewbox[3], 0.f);
    glVertex3f(v->viewbox[2], v->viewbox[3], 0.f);
    glVertex3f(v->viewbox[2], v->viewbox[1], 0.f);
  glEnd();
  glPopAttrib();

  // render tile guide
  if (v->tilesize > 0 && v->draw_tile == 1) {
    glTranslatef(0.f, 0.f, 0.2f); 
    glPushAttrib(GL_CURRENT_BIT);
    glLineStipple(1, 0x3333);
    glColor3f(.5f, .5f, .5f);
    GlDrawTileGuide(xviewsize, yviewsize, v->tilesize);
    glPopAttrib();
  }

  // Not swapping the buffers here is intentional.
}

void FbvResize(FrameBufferViewer *v, int width, int height)
{
  v->win_width = width;
  v->win_height = height;

  glViewport(0, 0, v->win_width, v->win_height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-.5f * v->win_width, .5f * v->win_width,
          -.5f * v->win_height, .5f * v->win_height,
          -1.f, 1.f);
}

void FbvPressButton(FrameBufferViewer *v, MouseButton button, int x, int y)
{
  v->xpresspos = x;
  v->ypresspos = y;

  switch (button) {
  case MB_LEFT:
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
      view_width = v->viewbox[2] - v->viewbox[0];
      bufx = (int) wx - v->viewbox[0];
      bufy = (int) wy - v->viewbox[1] * view_width;
      //bufy = (v->databox[3]-v->databox[1]-1) - bufy;
      //printf ("++++++(%d, %d)\n", bufx, (v->databox[3]-v->databox[1]-1) - bufy);
      bufy = FbGetHeight(v->fb) - bufy - 1;
      //printf ("------(%d, %d)\n", bufx, bufy);
      //bufx = CLAMP(bufx, 0, v->databox[2]-v->databox[0]-1);
      //bufy = CLAMP(bufy, 0, v->databox[3]-v->databox[1]-1);
      bufx = CLAMP(bufx, 0, FbGetWidth(v->fb)-1);
      bufy = CLAMP(bufy, 0, FbGetHeight(v->fb)-1);
      //printf ("(%d, %d)\n", bufx, bufy);
      buf = FbGetReadOnly(v->fb, (int) bufx, (int) bufy, 0);
      memcpy(pixel, buf, sizeof(float) * FbGetChannelCount(v->fb));
      printf("R:%g G:%g B:%g A:%g\n", pixel[0], pixel[1], pixel[2], pixel[3]);
        }
#endif
    break;
  case MB_MIDDLE:
    v->pressbutton = MB_MIDDLE;
    v->xlockoffset = v->xoffset;
    v->ylockoffset = v->yoffset;
    v->dist_per_pixel = 1.f/v->scale;
    break;
  case MB_RIGHT:
    v->pressbutton = MB_RIGHT;
    v->lockexponent = v->exponent;
    break;
  default:
    break;
  }
}

void FbvReleaseButton(FrameBufferViewer *v, MouseButton button, int x, int y)
{
  v->pressbutton = MB_NONE;
}

void FbvMoveMouse(FrameBufferViewer *v, int x, int y)
{
  const int posx = x;
  const int posy = y;

  switch (v->pressbutton) {
  case MB_MIDDLE:
    v->xoffset = v->xlockoffset + v->dist_per_pixel * (posx - v->xpresspos);
    v->yoffset = v->ylockoffset - v->dist_per_pixel * (posy - v->ypresspos);
    break;
  case MB_RIGHT:
    v->exponent = v->lockexponent + .01f * (float)(
          (posx - v->xpresspos) -
          (posy - v->ypresspos));
    v->exponent = Clamp(v->exponent, -5.f, 10.f);
    v->scale = pow(2, v->exponent);
    break;
  default:
    break;
  }
}

void FbvPressKey(FrameBufferViewer *v, unsigned char key, int mouse_x, int mouse_y)
{
  switch (key) {
  case 'h':
    set_to_home_position(v);
    FbvDraw(v);
    break;
  case 'r':
    v->diplay_channel = ( v->diplay_channel == DISPLAY_R ) ? DISPLAY_RGB : DISPLAY_R;
    setup_image_drawing(v);
    FbvDraw(v);
    break;
  case 'g':
    v->diplay_channel = ( v->diplay_channel == DISPLAY_G ) ? DISPLAY_RGB : DISPLAY_G;
    setup_image_drawing(v);
    FbvDraw(v);
    break;
  case 'b':
    v->diplay_channel = ( v->diplay_channel == DISPLAY_B ) ? DISPLAY_RGB : DISPLAY_B;
    setup_image_drawing(v);
    FbvDraw(v);
    break;
  case 'a':
    v->diplay_channel = ( v->diplay_channel == DISPLAY_A ) ? DISPLAY_RGB : DISPLAY_A;
    setup_image_drawing(v);
    FbvDraw(v);
    break;
  case 't':
    v->draw_tile = (v->draw_tile == 1) ? 0 : 1;
    FbvDraw(v);
    break;
  case 'u':
    FbvLoadImage(v, v->filename);
    setup_image_drawing(v);
    FbvDraw(v);
    break;
  case '\033': // ESC ASCII code
    exit(EXIT_SUCCESS);
    break;
  default:
    break;
  }
}

int FbvLoadImage(FrameBufferViewer *v, const char *filename)
{
  char try_filename[1024] = {'\0'};
  const size_t MAXCPY = 1024-1;
  const char *ext = NULL;
  int err = 0;

  StrCopyAndTerminate(try_filename, filename, MAXCPY);
  if (strcmp(v->filename, try_filename) != 0) {
    StrCopyAndTerminate(v->filename, try_filename, MAXCPY);
  }

  ext = file_extension(v->filename);
  if (ext == NULL) {
    return -1;
  }

  if (v->fb == NULL) {
    v->fb = FbNew();
  }

  if (strcmp(ext, "fb") == 0) {
    BufferInfo info = BUFINFO_INIT;
    err = load_fb(v->filename, v->fb, &info);
    BOX2_COPY(v->viewbox, info.viewbox);
    BOX2_COPY(v->databox, info.databox);
    v->tilesize = info.tilesize;
  }
  else if (strcmp(ext, "mip") == 0) {
    BufferInfo info = BUFINFO_INIT;
    err = load_mip(v->filename, v->fb, &info);
    BOX2_COPY(v->viewbox, info.viewbox);
    BOX2_COPY(v->databox, info.databox);
    v->tilesize = info.tilesize;
  }
  else {
    return -1;
  }
  if (err) {
    return -1;
  }

  {
    // TODO define gamma function
    float *pixel = v->fb->GetWritable(0, 0, 0);
    const int N = v->fb->GetWidth() * v->fb->GetHeight() * v->fb->GetChannelCount();
    int i;

    const float gamma = 1 / 2.2;

    for (i = 0; i < N; i++) {
      pixel[i] = pow(pixel[i], gamma);
    }
  }

  setup_image_drawing(v);

  return err;
}

void FbvGetImageSize(const FrameBufferViewer *v,
    int databox[4], int viewbox[4], int *nchannels)
{
  BOX2_COPY(databox, v->databox);
  BOX2_COPY(viewbox, v->viewbox);
  *nchannels = v->fb->GetChannelCount();
}

//----------------------------------------------------------------------------
static void setup_image_drawing(FrameBufferViewer *v)
{
  setup_image_drawer(&v->image, v->fb->GetReadOnly(0, 0, 0),
      v->fb->GetChannelCount(), v->diplay_channel,
      v->databox[0],
      v->viewbox[3] - v->viewbox[1] - v->databox[3],
      v->databox[2] - v->databox[0],
      v->databox[3] - v->databox[1]);
}

static void set_to_home_position(FrameBufferViewer *v)
{
  v->scale = 1.f;
  v->exponent = 0.f;
  v->lockexponent = 0.f;
  v->dist_per_pixel = 0.f;
  v->xoffset = 0.f;
  v->yoffset = 0.f;
  v->xpresspos = 0;
  v->ypresspos = 0;
  v->xlockoffset = 0.f;
  v->ylockoffset = 0.f;
}

void GlDrawTileGuide(int width, int height, int tilesize)
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
