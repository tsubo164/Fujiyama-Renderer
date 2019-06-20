// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_box.h"
#include "fj_pto.h"

#include <fstream>
#include <cerrno>

namespace fj {

static int error_no = ERR_FB_NOERR;

int FbGetErrorNo(void)
{
  return error_no;
}

const char *FbGetErrorMessage(int err)
{
  static const char *errmsg[] = {
    "",                      // ERR_FB_NOERR
    "No memory for FbInput", // ERR_FB_NOMEM
    "No such file",          // ERR_FB_NOFILE
    "Not framebuffer file",  // ERR_FB_NOTFB
    "Invalid file version"   // ERR_FB_BADVER
  };
  static const int nerrs = (int )sizeof(errmsg)/sizeof(errmsg[0]);

  if (err >= nerrs) {
    fprintf(stderr, "fatal error: error no %d is out of range\n", err);
    abort();
  }
  return errmsg[err];
}

static void set_error(int err)
{
  error_no = err;
}

int WriteFrameBuffer(const std::string &filename, const FrameBuffer &fb)
{
  set_error(ERR_FB_NOERR);
  std::ofstream strm(filename.c_str());
  if (!strm) {
    return -1;
  }

  WritePtoHeader(strm);
  strm << "#Fujiyama Renderer FrameBuffer\n";
  strm << "resolution " << fb.GetWidth() << " " << fb.GetHeight() << '\n';
  strm << "channel_count " << fb.GetChannelCount() << '\n';
  strm << "begin pixels\n";
  for (int y = 0; y < fb.GetHeight(); y++) {
    for (int x = 0; x < fb.GetWidth(); x++) {
      const Color4 c = fb.GetColor(x, y);
      strm << c[0] << " " << c[1] << " " << c[2] << " " << c[3] << '\n';
    }
  }
  strm << "end pixels\n";

  return 0;
}

class PtoFrameBuffer : public PtoParser
{
public:
  PtoFrameBuffer(FrameBuffer &fb) :
    fb_(fb), width_(0), height_(0), nchannels_(0), is_pixels_(false), x(0), y(0) {}
  virtual ~PtoFrameBuffer() {}

private:
  FrameBuffer &fb_;
  int width_, height_, nchannels_;
  bool is_pixels_;
  int x, y;

  virtual void begin(const std::vector<std::string> &tokens)
  {
    if (tokens.size() == 2 && tokens[1] == "pixels") {
      is_pixels_ = true;
      x = 0;
      y = 0;
    }
  }
  virtual void end(const std::vector<std::string> &tokens)
  {
    if (tokens.size() == 2 && tokens[1] == "pixels") {
      is_pixels_ = false;
    }
  }
  virtual void process(const std::vector<std::string> &tokens)
  {
    if (tokens[0] == "resolution" && tokens.size() == 3) {
      width_ = to_integer(tokens[1], 0);
      height_ = to_integer(tokens[2], 0);
    }
    else if (tokens[0] == "channel_count" && tokens.size() == 2) {
      nchannels_ = to_integer(tokens[1], 0);

      if (width_ > 0 && height_ > 0 && nchannels_ > 0) {
        fb_.Resize(width_, height_, nchannels_);
      }
    }
    else if (is_pixels_) {
      const Real r = to_float(tokens[0], 0);
      const Real g = to_float(tokens[1], 0);
      const Real b = to_float(tokens[2], 0);
      const Real a = to_float(tokens[3], 0);
      fb_.SetColor(x, y, Color4(r, g, b, a));
      if (++x == width_) {
        x = 0;
        y++;
      }
    }
  }
};

int ReadFrameBuffer(const std::string &filename, FrameBuffer &fb)
{
  std::ifstream strm(filename.c_str());
  PtoFrameBuffer pto(fb);

  set_error(ERR_FB_NOERR);
  pto.Parse(strm);

  return 0;
}

} // namespace xxx
