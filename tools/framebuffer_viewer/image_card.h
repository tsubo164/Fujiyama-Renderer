// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef IMAGE_CARD_H
#define IMAGE_CARD_H

#include "shader_program.h"

enum {
  DISPLAY_RGB = -1,
  DISPLAY_R = 0,
  DISPLAY_G = 1,
  DISPLAY_B = 2,
  DISPLAY_A = 3
};

class ImageCard {
public:
  ImageCard();
  ~ImageCard();

  void Init(const float *pixels, int channel_count, int display_channel,
      int xoffset, int yoffset, int xsize, int ysize);

  void Draw() const;
  void DrawOutline() const;

private:
  const float *pixels_;
  int display_channel_;
  int channel_count_;

  int xmin_, ymin_, xmax_, ymax_;
  ShaderProgram shader_program_;
};

#endif // XXX_H
