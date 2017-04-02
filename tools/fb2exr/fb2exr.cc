// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_color.h"
#include <ImfChannelList.h>
#include <ImfOutputFile.h>
#include <ImfRgbaFile.h>
#include <ImfHeader.h>
#include <ImfRgba.h>

#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

using namespace fj;

static const char USAGE[] =
"Usage: fb2exr [options] inputfile(*.fb) outputfile(*.exr)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

static void copy_fb_into_rgba(const FrameBuffer &fb, Imf::Rgba* rgba);
static void write_rgba(const char *filename, const Imf::Rgba *pixels, int width, int height);

int main(int argc, const char** argv)
try {
  using namespace std;

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    cout << USAGE;
    return 0;
  }

  if (argc != 3) {
    cerr << "error: invalid number of arguments\n";
    cerr << USAGE;
    return -1;
  }

  const std::string filename(argv[1]);
  fj::FrameBuffer fb;
  ReadFrameBuffer(filename, fb);

  vector<Imf::Rgba> rgba(fb.GetWidth() * fb.GetHeight());
  copy_fb_into_rgba(fb, &rgba[0]);
  write_rgba(argv[2], &rgba[0], fb.GetWidth(), fb.GetHeight());

  return 0;
}
catch (const std::exception& e) {
  std::cerr << "fatal error: " << e.what() << "\n";
  return -1;
}

static void copy_fb_into_rgba(const FrameBuffer &fb, Imf::Rgba* rgba)
{
  const int W = fb.GetWidth();
  const int H = fb.GetHeight();
  const int C = fb.GetChannelCount();

  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      const int index = y * W + x;
      const Color4 color = fb.GetColor(x, y);
      switch (C) {
      case 1:
        rgba[index].r = color[0];
        rgba[index].g = color[0];
        rgba[index].b = color[0];
        rgba[index].a = 1;
        break;
      case 3:
        rgba[index].r = color[0];
        rgba[index].g = color[1];
        rgba[index].b = color[2];
        rgba[index].a = 1;
        break;
      case 4:
        rgba[index].r = color[0];
        rgba[index].g = color[1];
        rgba[index].b = color[2];
        rgba[index].a = color[3];
        break;
      default:
        rgba[index].r = 0;
        rgba[index].g = 0;
        rgba[index].b = 0;
        rgba[index].a = 0;
        break;
      }
    }
  }
}

static void write_rgba(const char *filename, const Imf::Rgba *pixels, int width, int height)
{
  Imf::RgbaOutputFile file(filename, width, height, Imf::WRITE_RGBA);
  file.setFrameBuffer(pixels, 1, width);
  file.writePixels(height);
}
