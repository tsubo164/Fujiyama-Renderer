// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "draw_image.h"
#include <stddef.h>
#include <assert.h>

ImageCard::ImageCard() :
    pixels_(NULL),
    display_channel_(DISPLAY_RGB),
    channel_count_(4),
    xmin_(0),
    ymin_(0),
    xmax_(0),
    ymax_(0)
{
}

ImageCard::~ImageCard()
{
}

void ImageCard::Init(const float *pixels, int channel_count, int display_channel,
    int xoffset, int yoffset, int xsize, int ysize)
{
  GLenum format = 0;

  pixels_ = pixels;
  channel_count_ = channel_count;
  display_channel_ = display_channel;
  xmin_ = xoffset;
  ymin_ = yoffset;
  xmax_ = xoffset + xsize;
  ymax_ = yoffset + ysize;

  switch (channel_count_) {
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
  glPixelStorei(GL_UNPACK_ALIGNMENT, channel_count_);
  glTexImage2D(GL_TEXTURE_2D, 0, format, xsize, ysize, 0,
          format, GL_FLOAT, pixels_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  if (shader_program_.GetProgramID() == 0) {
    shader_program_.Init();
  }
}

void ImageCard::Draw() const
{
  if (pixels_ == NULL) {
    return;
  }

  glColor3f(1.f, 1.f, 1.f);

  glUseProgram(shader_program_.GetProgramID());
  shader_program_.SetUniformInt("texture", 0);
  shader_program_.SetUniformInt("display_channels", display_channel_);
  shader_program_.SetUniformInt("channel_count",    channel_count_);

  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
    glTexCoord2f(0.f, 1.f); glVertex3f(xmin_, ymin_, 0.f);
    glTexCoord2f(1.f, 1.f); glVertex3f(xmax_, ymin_, 0.f);
    glTexCoord2f(1.f, 0.f); glVertex3f(xmax_, ymax_, 0.f);
    glTexCoord2f(0.f, 0.f); glVertex3f(xmin_, ymax_, 0.f);
  glEnd();
  glDisable(GL_TEXTURE_2D);

  glUseProgram(0);
}

void ImageCard::DrawOutline() const
{
  glPushAttrib(GL_CURRENT_BIT);
  glEnable(GL_LINE_STIPPLE);
  glColor3f(0.3f, .3f, .3f);
  glLineStipple(1, 0x3333);
  glBegin(GL_LINE_LOOP);
    glVertex3f(xmin_, ymin_, 0.f);
    glVertex3f(xmax_, ymin_, 0.f);
    glVertex3f(xmax_, ymax_, 0.f);
    glVertex3f(xmin_, ymax_, 0.f);
  glEnd();
  glPopAttrib();
}
