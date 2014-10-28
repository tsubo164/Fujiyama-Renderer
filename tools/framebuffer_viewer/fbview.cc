// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "framebuffer_viewer.h"
#include "compatible_opengl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace fj;

static const char USAGE[] =
"Usage: fbview [options] file(*.fb, *.mip)\n"
"Options:\n"
"  --help            Display this information\n"
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
"  ESC or q          Quit Application\n"
"\n";

static FrameBufferViewer *viewer = NULL;
static void exit_viewer(void);
static void display(void);
static void resize(int w, int h);
static void mouse(int button, int state, int x, int y);
static void motion(int x, int y);
static void keyboard(unsigned char key, int x, int y);
static void timer(int value);

static int initialize_viewer(const char *filename);

int main(int argc, char **argv)
{
  const char *filename = NULL;
  const int WIN_W = 660;
  const int WIN_H = 500;
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
  glutInitWindowSize(WIN_W, WIN_H);
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

static int initialize_viewer(const char *filename)
{
  int databox[4] = {0, 0, 0, 0};
  int viewbox[4] = {0, 0, 0, 0};

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
      viewer->GetImageSize(databox, viewbox, &nchannels);
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
      printf("%d x %d: %s\n", viewbox[2]-viewbox[0], viewbox[3]-viewbox[1], format);
      printf("databox: %d %d %d %d\n", databox[0], databox[1], databox[2], databox[3]);
      printf("viewbox: %d %d %d %d\n", viewbox[0], viewbox[1], viewbox[2], viewbox[3]);
    }
  }

  return 0;
}
