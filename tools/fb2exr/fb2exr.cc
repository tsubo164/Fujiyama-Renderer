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
static void write_rgba_layer(const char *filename,
    const Imf::Rgba* pixels, const Imath::Box2i &dispwin, const Imath::Box2i &datawin);
static Imath::Box2i make_box2i(int *box);

int main(int argc, const char** argv)
try {
  using namespace std;
  //using namespace Imf;
  //using ::FrameBuffer;

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    cout << USAGE;
    return 0;
  }

  if (argc != 3) {
    cerr << "error: invalid number of arguments\n";
    cerr << USAGE;
    return -1;
  }

  const string in_file(argv[1]);
  FbInput *in = FbOpenInputFile(in_file.c_str());
  if (in == NULL) {
    cerr << "error: could not open input file: " << in_file << "\n";
    return -1;
  }

  if (FbReadHeader(in)) {
    cerr << "error: failed to read header: " << in_file << "\n";
    FbCloseInputFile(in);
    return -1;
  }
  const Imath::Box2i dispwin = make_box2i(in->viewbox);
  const Imath::Box2i datawin = make_box2i(in->databox);

  fj::FrameBuffer fb;
  fb.Resize(in->width, in->height, in->nchannels);
  in->data = fb.GetWritable(0, 0, 0);

  FbReadData(in);
  FbCloseInputFile(in);

  vector<Imf::Rgba> rgba(fb.GetWidth() * fb.GetHeight());
  copy_fb_into_rgba(fb, &rgba[0]);

  write_rgba_layer(argv[2], &rgba[0], dispwin, datawin);

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

static void write_rgba_layer(const char *filename,
    const Imf::Rgba* pixels, const Imath::Box2i &dispwin, const Imath::Box2i &datawin)
{
  using namespace Imf;

  const int DISP_WIDTH  = dispwin.max.x - dispwin.min.x + 1;
  const int DISP_HEIGHT = dispwin.max.y - dispwin.min.y + 1;

  const int DATA_WIDTH = datawin.max.x - datawin.min.x + 1;
  const int DATA_HEIGHT = datawin.max.y - datawin.min.y + 1;
  const Imf::Rgba* BASE = pixels - datawin.min.x - datawin.min.y * DATA_WIDTH;

  Imf::Header header(DISP_WIDTH, DISP_HEIGHT);
  header.dataWindow() = datawin;
  header.channels().insert("R", Imf::Channel(Imf::HALF));
  header.channels().insert("G", Imf::Channel(Imf::HALF));
  header.channels().insert("B", Imf::Channel(Imf::HALF));
  header.channels().insert("A", Imf::Channel(Imf::HALF));

  Imf::OutputFile exr(filename, header);
  Imf::FrameBuffer frameBuffer;

  frameBuffer.insert("R",               // name
      Imf::Slice(Imf::HALF,             // type
          (char *)(&BASE->r),           // base
          sizeof(*BASE),                // xStride
          sizeof(*BASE) * DATA_WIDTH)); // yStride

  frameBuffer.insert("G",               // name
      Imf::Slice(Imf::HALF,             // type
          (char *)(&BASE->g),           // base
          sizeof(*BASE),                // xStride
          sizeof(*BASE) * DATA_WIDTH)); // yStride

  frameBuffer.insert("B",               // name
      Imf::Slice(Imf::HALF,             // type
          (char *)(&BASE->b),           // base
          sizeof(*BASE),                // xStride
          sizeof(*BASE) * DATA_WIDTH)); // yStride

  frameBuffer.insert("A",               // name
      Imf::Slice(Imf::HALF,             // type
          (char *)(&BASE->a),           // base
          sizeof(*BASE),                // xStride
          sizeof(*BASE) * DATA_WIDTH)); // yStride

  exr.setFrameBuffer(frameBuffer);
  exr.writePixels(DATA_HEIGHT);
}

static Imath::Box2i make_box2i(int *box)
{
  return Imath::Box2i(Imath::V2i(box[0], box[1]), Imath::V2i(box[2] - 1, box[3] - 1));
}
