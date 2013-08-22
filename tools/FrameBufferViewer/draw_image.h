/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef DRAW_IMAGE_H
#define DRAW_IMAGE_H

#include "glsl_shaders.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
  DISPLAY_RGB = -1,
  DISPLAY_R = 0,
  DISPLAY_G = 1,
  DISPLAY_B = 2,
  DISPLAY_A = 3
};

struct ImageCard {
  const float *pixels;
  int display_channel;
  int channel_count;

  int xmin, ymin, xmax, ymax;
  struct ShaderProgram shader_program;
};

extern void init_image_drawer(struct ImageCard *image);

extern void setup_image_drawer(struct ImageCard *image, const float *pixels,
    int channel_count, int display_channel,
    int xoffset, int yoffset, int xsize, int ysize);

extern void draw_image(const struct ImageCard *image);

extern void draw_outline(const struct ImageCard *image);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */
