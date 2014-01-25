/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#if defined(FJ_WINDOWS)
#define OPENEXR_DLL
#endif

#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
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

static const char USAGE[] =
"Usage: fb2exr [options] inputfile(*.fb) outputfile(*.exr)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

static void copy_fb_into_rgba(const float* src, Imf::Rgba* dst,
    int width, int height, int nchannels);
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
  struct FbInput *in = FbOpenInputFile(in_file.c_str());
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

  struct ::FrameBuffer *fb = FbNew();
  if (fb == NULL) {
    FbCloseInputFile(in);
    throw runtime_error("could not create FrameBuffer");
    return -1;
  }

  FbResize(fb, in->width, in->height, in->nchannels);
  in->data = FbGetWritable(fb, 0, 0, 0);

  FbReadData(in);
  FbCloseInputFile(in);

  const int width = FbGetWidth(fb);
  const int height = FbGetHeight(fb);
  const int nchannels = FbGetChannelCount(fb);
  vector<Imf::Rgba> rgba(width * height);

  copy_fb_into_rgba(FbGetReadOnly(fb, 0, 0, 0), &rgba[0], width, height, nchannels);

  FbFree(fb);

  write_rgba_layer(argv[2], &rgba[0], dispwin, datawin);

  return 0;
}
catch (const std::exception& e) {
  std::cerr << "fatal error: " << e.what() << "\n";
  return -1;
}

static void copy_fb_into_rgba(const float* src, Imf::Rgba* dst,
    int width, int height, int nchannels)
{
  for (int i = 0; i < width * height; i++) {
    dst[i].r = src[i*nchannels + 0];
    dst[i].g = src[i*nchannels + 1];
    dst[i].b = src[i*nchannels + 2];
    if (nchannels == 4)
      dst[i].a = src[i*nchannels + 3];
    else
      dst[i].a = 1;
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
