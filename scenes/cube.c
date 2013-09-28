/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

/*
example scene of C interfaces
1 cube with 1 point lights

to compile and render this scene, run this at the top level of source tree

 $ make sample

*/

#include "fj_scene_interface.h"
#include <stdio.h>

#define USE_CUSTOM_CALLBACKS 0

#if USE_CUSTOM_CALLBACKS
static Interrupt frame_start(void *data, const struct FrameInfo *info)
{
  printf("# Callback Sample -- Frame Start\n");
  return CALLBACK_CONTINUE;
}
static Interrupt frame_done(void *data, const struct FrameInfo *info)
{
  printf("# Callback Sample -- Frame Done\n");
  return CALLBACK_CONTINUE;
}

static int N = 0;
static Interrupt tile_start(void *data, const struct TileInfo *info)
{
  printf("#   Callback Sample -- Tile Start\n");
  return CALLBACK_CONTINUE;
}
static Interrupt interrupt_in_the_middle(void *data)
{
  int *n = (int *) data;
  if (*n == 320 * 240 * 3 * 3 / 2)
    return CALLBACK_INTERRUPT;
  (*n)++;
  return CALLBACK_CONTINUE;
}
static Interrupt tile_done(void *data, const struct TileInfo *info)
{
  printf("#   Callback Sample -- Tile Done\n");
  return CALLBACK_CONTINUE;
}
#endif /* USE_CUSTOM_CALLBACKS */

int main(int argc, const char **argv)
{
  const int W = 320;
  const int H = 240;

  ID framebuffer;
  ID renderer;
  ID camera;
  ID object;
  ID shader;
  ID light;
  ID mesh;

  /* Scene */
  SiOpenScene();

  /* Plugin */
  if (SiOpenPlugin("PlasticShader")) {
    /* TODO error handling */
    /*
    fprintf(stderr, "Could not open shader: %s\n", SiGetErrorMessage(SiGetErrorNo()));
    */
  }

  /* Camera */
  camera = SiNewCamera("PerspectiveCamera");
  if (camera == SI_BADID) {
    fprintf(stderr, "Could not allocate camera\n");
    return -1;
  }
  SiSetProperty3(camera, "translate", 3, 3, 3);
  SiSetProperty3(camera, "rotate", -35.264389682754654, 45, 0);

  /* Light */
  light = SiNewLight(SI_POINT_LIGHT);
  if (light  == SI_BADID) {
    fprintf(stderr, "Could not allocate light\n");
    return -1;
  }
  SiSetProperty3(light, "translate", 1, 12, 3);

  /* Shader */
  shader = SiNewShader("PlasticShader");
  if (shader == SI_BADID) {
    fprintf(stderr, "Could not create shader: PlasticShader\n");
    return -1;
  }

  /* Mesh and Accelerator */
  mesh = SiNewMesh("cube.mesh");
  if (mesh == SI_BADID) {
    /* TODO error handling */
    /*
    fprintf(stderr, "Could not create mesh: %s\n", SiGetErrorMessage(SiGetErrorNo()));
    */
    return -1;
  }

  /* ObjectInstance */
  object = SiNewObjectInstance(mesh);
  if (object == SI_BADID) {
    fprintf(stderr, "Could not create object instance\n");
    return -1;
  }
  SiSetProperty3(object, "rotate", 0, 10, 0);
  SiAssignShader(object, shader);

  /* FrameBuffer */
  framebuffer = SiNewFrameBuffer("rgba");
  if (framebuffer == SI_BADID) {
    fprintf(stderr, "Could not allocate framebuffer\n");
    return -1;
  }

  /* Renderer */
  renderer = SiNewRenderer();
  if (renderer == SI_BADID) {
    fprintf(stderr, "Could not allocate renderer\n");
    return -1;
  }
  SiSetProperty2(renderer, "resolution", W, H);
  SiAssignCamera(renderer, camera);
  SiAssignFrameBuffer(renderer, framebuffer);

#if USE_CUSTOM_CALLBACKS
  SiSetFrameReportCallback(renderer,
      NULL,
      frame_start,
      frame_done);
  SiSetTileReportCallback(renderer,
      &N,
      tile_start,
      interrupt_in_the_middle,
      tile_done);
#endif /* USE_CUSTOM_CALLBACKS */

  /* Render scene */
  SiRenderScene(renderer);
  SiSaveFrameBuffer(framebuffer, "cube.fb");
  SiCloseScene();

  return 0;
}
