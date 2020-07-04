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
"    h   Back to home position\n"
"    r   Display red channel. one more hit back to rgb\n"
"    g   Display green channel. one more hit back to rgb\n"
"    b   Display blue channel. one more hit back to rgb\n"
"    a   Display alpha channel. one more hit back to rgb\n"
"    t   Toggle displaying tile guide lines when viewing *.mip\n"
"    u   Update (reload) image file\n"
"    l   Toggle listening mode on/off\n"
"    q   Quit Application\n"
"  ESC   Abort render process when displaying progress\n"
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
static void render_status_message(const FrameBufferViewer *viewer);

static const int WINDOW_WIDTH = 1920 / 2;
static const int WINDOW_HEIGHT = 1080 / 2;

int main(int argc, char **argv)
{
  const char *filename = NULL;
  const char *win_title = "FrameBuffer Viewer";

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    std::cout << USAGE << USAGE2;
    return 0;
  }

  if (argc != 2) {
    std::cerr << "ERROR: Invalid number of arguments.\n\n";
    std::cerr << USAGE << USAGE2;
    return -1;
  }

  if (strlen(argv[1]) > 1000) {
    std::cerr << "ERROR: Too long file name.\n\n";
    return -1;
  }

  if (strcmp(argv[1], "--listen") == 0) {
    filename = NULL;
  } else {
    filename = argv[1];
  }

  glutInit(&argc, argv);
  // XXX on maxOS, need to reszie window after creation to viewport work properly
  // for some reason. so create window with nimus 1 pixel then resize back to normal.
  glutInitWindowSize(WINDOW_WIDTH - 1, WINDOW_HEIGHT);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(win_title);

  // XXX on maxOS, need to reszie window after creation to viewport work properly
  // for some reason. so create window with nimus 1 pixel then resize back to normal.
  glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

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
      std::cerr << "ERROR: " << glewGetErrorString(err) << "\n\n";
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

  render_status_message(viewer);

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
  // avoid windows Rectangle name conflict
  fj::Rectangle viewbox;

  // create viewer
  viewer = new FrameBufferViewer();
  if (viewer == NULL) {
    std::cerr << "Could not allocate FrameBufferViewer\n\n";
    return -1;
  }

  // register cleanup function
  if (atexit(exit_viewer) != 0) {
    std::cerr << "Could not register viewer_exit()\n\n";
  }

  if (filename == NULL) {
    viewer->StartListening();
    return 0;
  }

  // load image
  if (viewer->LoadImage(filename)) {
    std::cerr << "Could not open framebuffer file: " << filename << "\n\n";
    return -1;
  }

  return 0;
}

static void render_status_message(const FrameBufferViewer *viewer)
{
  void *font = GLUT_BITMAP_HELVETICA_18;
  const std::string &text = viewer->GetStatusMessage();
  int width, height;
  viewer->GetWindowSize(width, height);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // to top left corner
  glTranslatef(-width / 2, height / 2, 0);
  // offset a bit
  glTranslatef(12, -20, 0);

  // drop shadow
  glColor3f(0, 0, 0);
  glRasterPos3f(1, -1, 0);
  for (char c: text) {
    glutBitmapCharacter(font, c);
  }

  // main text
  glColor3f(1, 1, 1);
  glRasterPos3f(0, 0, 0);
  for (char c: text) {
    glutBitmapCharacter(font, c);
  }
}
