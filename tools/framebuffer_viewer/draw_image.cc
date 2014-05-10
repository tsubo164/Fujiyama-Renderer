/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "draw_image.h"
#include <stddef.h>
#include <assert.h>

void init_image_drawer(struct ImageCard *image)
{
  image->pixels = NULL;
  image->display_channel = DISPLAY_RGB;
  image->channel_count = 4;
  image->xmin = 0;
  image->ymin = 0;
  image->xmax = 0;
  image->ymax = 0;

  image->shader_program.vert_shader_id = 0;
  image->shader_program.frag_shader_id = 0;
  image->shader_program.program_id = 0;
}

void setup_image_drawer(struct ImageCard *image, const float *pixels,
    int channel_count, int display_channel,
    int xoffset, int yoffset, int xsize, int ysize)
{
  GLenum format = 0;

  image->pixels = pixels;
  image->channel_count = channel_count;
  image->display_channel = display_channel;
  image->xmin = xoffset;
  image->ymin = yoffset;
  image->xmax = xoffset + xsize;
  image->ymax = yoffset + ysize;

  switch (image->channel_count) {
  case 1:
    format = GL_RED;
    break;
  case 3:
    format = GL_RGB;
    break;
  case 4:
    format = GL_RGBA;
    break;
  default:
    assert(!"invalid channel count");
    break;
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, image->channel_count);
  glTexImage2D(GL_TEXTURE_2D, 0, format, xsize, ysize, 0,
          format, GL_FLOAT, image->pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  if (image->shader_program.program_id == 0) {
    init_shaders(&image->shader_program);
  }
}

void draw_image(const struct ImageCard *image)
{
  if (image->pixels == NULL) {
    return;
  }

  glColor3f(1.f, 1.f, 1.f);

  glUseProgram(image->shader_program.program_id);
  set_uniform_int(&image->shader_program, "texture", 0);
  set_uniform_int(&image->shader_program, "display_channels", image->display_channel);
  set_uniform_int(&image->shader_program, "channel_count",    image->channel_count);

  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
    glTexCoord2f(0.f, 1.f); glVertex3f(image->xmin, image->ymin, 0.f);
    glTexCoord2f(1.f, 1.f); glVertex3f(image->xmax, image->ymin, 0.f);
    glTexCoord2f(1.f, 0.f); glVertex3f(image->xmax, image->ymax, 0.f);
    glTexCoord2f(0.f, 0.f); glVertex3f(image->xmin, image->ymax, 0.f);
  glEnd();
  glDisable(GL_TEXTURE_2D);

  glUseProgram(0);
}

void draw_outline(const struct ImageCard *image)
{
  glPushAttrib(GL_CURRENT_BIT);
  glEnable(GL_LINE_STIPPLE);
  glColor3f(0.3f, .3f, .3f);
  glLineStipple(1, 0x3333);
  glBegin(GL_LINE_LOOP);
    glVertex3f(image->xmin, image->ymin, 0.f);
    glVertex3f(image->xmax, image->ymin, 0.f);
    glVertex3f(image->xmax, image->ymax, 0.f);
    glVertex3f(image->xmin, image->ymax, 0.f);
  glEnd();
  glPopAttrib();
}
