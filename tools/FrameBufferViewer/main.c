/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "FrameBufferViewer.h"
#include "CompatibleOpenGL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
"  ESC               Quit Application\n"
"\n";

static struct FrameBufferViewer *viewer = NULL;
static void exit_viewer(void);
static void display(void);
static void resize(int w, int h);
static void mouse(int button, int state, int x, int y);
static void motion(int x, int y);
static void keyboard(unsigned char key, int x, int y);

static int initialize_viewer(const char *filename);

int main(int argc, char **argv)
{
  const int WIN_W = 256;
  const int WIN_H = 256;

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    fprintf(stderr, "%s%s", USAGE, USAGE2);
    return 0;
  }

  if (argc != 2) {
    fprintf(stderr, "error: invalid number of arguments.\n");
    fprintf(stderr, "%s%s", USAGE, USAGE2);
    return -1;
  }

  /* tipical glut settings */
  glutInit(&argc, argv);
  glutInitWindowSize(WIN_W, WIN_H);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow("FrameBuffer Viewer");

  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);

#if defined(SI_WINDOWS)
  {
    const GLenum err = glewInit();
    if (err != GLEW_OK) {
      /* Problem: glewInit failed, something is seriously wrong. */
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      return -1;
    }
  }
#endif

  if (initialize_viewer(argv[1])) {
    return -1;
  }

  glutMainLoop();

  return 0;
}

static void exit_viewer(void)
{
  FbvFreeViewer(viewer);
}

static void display(void)
{
  FbvDraw(viewer);
  glutSwapBuffers();
}

static void resize(int w, int h)
{
  FbvResize(viewer, w, h);
  glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN) {
    MouseButton btn = MB_NONE;
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
      btn = MB_LEFT;
      break;
    case GLUT_MIDDLE_BUTTON:
      btn = MB_MIDDLE;
      break;
    case GLUT_RIGHT_BUTTON:
      btn = MB_RIGHT;
      break;
    default:
      break;
    }
    FbvPressButton(viewer, btn, x, y);
  } else if (state == GLUT_UP) {
    FbvReleaseButton(viewer, MB_NONE, x, y);
  }
}

static void motion(int x, int y)
{
  FbvMoveMouse(viewer, x, y);
  glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y)
{
  FbvPressKey(viewer, key, x, y);
  glutPostRedisplay();
}

static int initialize_viewer(const char *filename)
{
  int databox[4] = {0, 0, 0, 0};
  int viewbox[4] = {0, 0, 0, 0};

  /* create viewer */
  viewer = FbvNewViewer();
  if (viewer == NULL) {
    fprintf(stderr, "Could not allocate FrameBufferViewer\n");
    return -1;
  }

  /* register cleanup function */
  if (atexit(exit_viewer) != 0) {
    fprintf(stderr, "Could not register viewer_exit()\n");
  }

  /* load image */
  {
    if (FbvLoadImage(viewer, filename)) {
      fprintf(stderr, "Could not open framebuffer file: %s\n", filename);
      return -1;
    } else {
      const char *format;
      int nchannels;
      /* get image size info */
      FbvGetImageSize(viewer, databox, viewbox, &nchannels);
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
