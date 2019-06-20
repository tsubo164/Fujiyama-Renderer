// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "framebuffer_viewer.h"
#include "compatible_opengl.h"
#include "fj_numeric.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace fj;

static const char USAGE[] =
"Usage: fbview [options] file(*.fb, *.mip)\n"
"Options:\n"
"  --help            Display this information\n"
"  --listen          Start viewer waiting for rendering\n"
"\n"
"Mouse Operations:\n"
"  Middle button     Move image\n"
"  Right button      Zoom image\n"
"\n";

static const char USAGE2[] =
"Hotkeys:\n"
"  h                 Back to home position\n"
"  r                 Display red channel. one more hit back to rgb\n"
"  g                 Display green channel. one more hit back to rgb\n"
"  b                 Display blue channel. one more hit back to rgb\n"
"  a                 Display alpha channel. one more hit back to rgb\n"
"  t                 Toggle displaying tile guide lines when viewing *.mip\n"
"  u                 Update (reload) image file\n"
"  l                 Toggle listening mode on/off\n"
"  q                 Quit Application\n"
"  ESC               Abort render process when displaying progress\n"
"\n";

static FrameBufferViewer *viewer = NULL;
static void exit_viewer(void);
static void display(void);
static void resize(int w, int h);
static void mouse(int button, int state, int x, int y);
static void motion(int x, int y);
static void keyboard(unsigned char key, int x, int y);
static void timer(int value);

// called back by framebuffer
static void window_resize_callback(void *win, int x_image_res, int y_image_res);
static void window_change_title_callback(void *win, const char *title);

static int initialize_viewer(const char *filename);

static const int WIN_W_MIN = 320;
static const int WIN_H_MIN = 240;
static const int WIN_W_MAX = 1280;
static const int WIN_H_MAX = 720;

int main(int argc, char **argv)
{
  const char *filename = NULL;
  char win_title[1024] = "FrameBuffer Viewer";

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    fprintf(stderr, "%s%s", USAGE, USAGE2);
    return 0;
  }

  if (argc != 2) {
    fprintf(stderr, "error: invalid number of arguments.\n");
    fprintf(stderr, "%s%s", USAGE, USAGE2);
    return -1;
  }

  if (strlen(argv[1]) > 1000) {
    fprintf(stderr, "error: too long file name.\n");
    return -1;
  }

  if (strcmp(argv[1], "--listen") == 0) {
    filename = NULL;
    sprintf(win_title, "Listen Mode - FrameBuffer Viewer");
  } else {
    filename = argv[1];
    sprintf(win_title, "%s - FrameBuffer Viewer", filename);
  }

  // tipical glut settings
  glutInit(&argc, argv);
  glutInitWindowSize(WIN_W_MIN, WIN_H_MIN);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(win_title);

  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  glutTimerFunc(1000, timer, 0);

#if defined(FJ_WINDOWS)
  {
    const GLenum err = glewInit();
    if (err != GLEW_OK) {
      // Problem: glewInit failed, something is seriously wrong.
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      return -1;
    }
  }
#endif

  if (initialize_viewer(filename)) {
    return -1;
  }

  glutMainLoop();

  return 0;
}

static void exit_viewer(void)
{
  delete viewer;
}

static void display(void)
{
  viewer->Draw();
  glutSwapBuffers();
}

static void resize(int w, int h)
{
  viewer->Resize(w, h);
  glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN) {
    MouseButton btn = MOUSE_BUTTON_NONE;
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
      btn = MOUSE_BUTTON_LEFT;
      break;
    case GLUT_MIDDLE_BUTTON:
      btn = MOUSE_BUTTON_MIDDLE;
      break;
    case GLUT_RIGHT_BUTTON:
      // TODO not sure why need cast only here
      btn = MOUSE_BUTTON_RIGHT;
      break;
    default:
      break;
    }
    viewer->PressButton(btn, x, y);
  } else if (state == GLUT_UP) {
    viewer->ReleaseButton(MOUSE_BUTTON_NONE, x, y);
  }
}

static void motion(int x, int y)
{
  viewer->MoveMouse(x, y);
  glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y)
{
  viewer->PressKey(key, x, y);
  glutPostRedisplay();

  // to start listening when key 'l' pressed.
  timer(0);
}

static void timer(int value)
{
  if (viewer->IsListening()) {
    viewer->Listen();
    glutPostRedisplay();

    const int interval = viewer->IsRendering() ? 1 : 100;
    glutTimerFunc(interval, timer, 0);
  }
}

static void window_resize_callback(void *win, int x_image_res, int y_image_res)
{
  const int window_margin = 20;
  const int new_window_size_x = x_image_res + 2 * window_margin;
  const int new_window_size_y = y_image_res + 2 * window_margin;
  glutReshapeWindow(
      Clamp(new_window_size_x, WIN_W_MIN, WIN_W_MAX),
      Clamp(new_window_size_y, WIN_H_MIN, WIN_H_MAX));
}

static void window_change_title_callback(void *win, const char *title)
{
  const std::string new_window_title(std::string(title) + " - FrameBuffer Viewer");
  glutSetWindowTitle(new_window_title.c_str());
}

static int initialize_viewer(const char *filename)
{
  // avoid windows Rectangle
  fj::Rectangle viewbox;

  // create viewer
  viewer = new FrameBufferViewer();
  if (viewer == NULL) {
    fprintf(stderr, "Could not allocate FrameBufferViewer\n");
    return -1;
  }

  // register cleanup function
  if (atexit(exit_viewer) != 0) {
    fprintf(stderr, "Could not register viewer_exit()\n");
  }

  // set callback functions
  viewer->SetWindowResizeRequest     (NULL, window_resize_callback);
  viewer->SetWindowChangeTitleRequest(NULL, window_change_title_callback);

  if (filename == NULL) {
    viewer->StartListening();
    return 0;
  }

  // load image
  {
    if (viewer->LoadImage(filename)) {
      fprintf(stderr, "Could not open framebuffer file: %s\n", filename);
      return -1;
    } else {
      const char *format;
      int nchannels;
      // get image size info
      viewer->GetImageSize(viewbox, &nchannels);
      switch (nchannels) {
      case 3:
        format = "RGB";
        break;
      case 4:
        format = "RGBA";
        break;
      default:
        format = "UNKNOWN";
        break;
      }
      printf("%d x %d: %s\n", viewbox.Size()[0], viewbox.Size()[1], format);
    }
  }

  return 0;
}
