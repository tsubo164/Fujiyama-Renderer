// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_mipmap.h"
#include "fj_framebuffer.h"
#include "fj_numeric.h"
#include "fj_memory.h"
#include "fj_vector.h"
#include "fj_box.h"

#include <string.h>
#include <assert.h>
#include <errno.h>
#include <math.h>

#define MIP_FILE_VERSION 1
#define MIP_FILE_MAGIC "MIPM"
#define MIP_MAGIC_SIZE 4

#define POW2_SIZE 16

#define ROUND(x_) (floor((x_)+.5))

namespace fj {

static int error_no = ERR_MIP_NOERR;
static int pow2[POW2_SIZE] = {
  1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
};

struct Coordinate { int x, y; };
struct Position { float x, y; };

static void compute_output_res(int in_w, int in_h, int *out_w, int *out_h);
static void scale_and_copy_image(const float *src, int sw, int sh,
    float *dst, int dw, int dh, int nchannels);

static void set_error(int err);

int MipGetErrorNo(void)
{
  return error_no;
}

const char *MipGetErrorMessage(int err)
{
  static const char *errmsg[] = {
    "",                       /* ERR_MIP_NOERR */
    "No memory for MipInput", /* ERR_MIP_NOMEM */
    "No such file",           /* ERR_MIP_NOFILE */
    "Not mipmap file",        /* ERR_MIP_NOTMIP */
    "Invalid file version"    /* ERR_MIP_BADVER */
  };
  static const int nerrs = (int) sizeof(errmsg)/sizeof(errmsg[0]);

  if (err >= nerrs) {
    fprintf(stderr, "fatal error: error no %d is out of range\n", err);
    abort();
  }
  return errmsg[err];
}

MipInput::MipInput() :
    file_(NULL),
    version_(0),
    width_(0),
    height_(0),
    nchannels_(0),
    tilesize_(0),
    xntiles_(0),
    yntiles_(0),
    offset_of_header_(0),
    offset_of_tile_(0)
{
}

MipInput::~MipInput()
{
  Close();
}

int MipInput::Open(const std::string &filename)
{
  file_ = fopen(filename.c_str(), "rb");
  if (file_ == NULL) {
    set_error(ERR_MIP_NOFILE);
    Close();
    return -1;
  }
  return 0;
}

void MipInput::Close()
{
  if (file_ != NULL) {
    fclose(file_);
    file_ = NULL;
  }
}

int MipInput::ReadHeader()
{
  size_t nreads = 0;
  char magic[MIP_MAGIC_SIZE];

  nreads += sizeof(char) * fread(magic, sizeof(char), MIP_MAGIC_SIZE, file_);
  if (memcmp(magic, MIP_FILE_MAGIC, MIP_MAGIC_SIZE) != 0) {
    set_error(ERR_MIP_NOTMIP);
    return -1;
  }
  nreads += sizeof(int) * fread(&version_, sizeof(int), 1, file_);
  if (version_ != MIP_FILE_VERSION) {
    set_error(ERR_MIP_BADVER);
    return -1;
  }
  nreads += sizeof(int) * fread(&width_, sizeof(int), 1, file_);
  nreads += sizeof(int) * fread(&height_, sizeof(int), 1, file_);
  nreads += sizeof(int) * fread(&nchannels_, sizeof(int), 1, file_);
  nreads += sizeof(int) * fread(&tilesize_, sizeof(int), 1, file_);

  xntiles_ = width_ / tilesize_;
  yntiles_ = height_ / tilesize_;

  offset_of_header_ = nreads;
  offset_of_tile_ = sizeof(float) * tilesize_ * tilesize_ * nchannels_;

  return 0;
}

int MipInput::ReadTile(int xtile, int ytile, float *dst)
{
  const int TILESIZE = tilesize_;
  const int XNTILES = width_ / TILESIZE;
  const int YNTILES = height_ / TILESIZE;
  const int TILE_PXLS = TILESIZE * TILESIZE * nchannels_;
  size_t nread = 0;

  // TODO TEMP out of border handling
  const int x = Clamp(xtile, 0, XNTILES-1);
  const int y = Clamp(ytile, 0, YNTILES-1);
  const int tile_index = y * XNTILES + x;

  fseek(file_, offset_of_header_ + sizeof(float) * tile_index * TILE_PXLS , SEEK_SET);

  nread = fread(dst, sizeof(float), TILE_PXLS, file_);
  if (nread == 0) {
    // TODO error handling
    return -1;
  }

  return 0;
}

bool MipInput::IsOpen() const
{
  return file_ != NULL;
}

struct MipOutput *MipOpenOutputFile(const char *filename)
{
  struct MipOutput *out = FJ_MEM_ALLOC(struct MipOutput);

  if (out == NULL) {
    set_error(ERR_MIP_NOMEM);
    return NULL;
  }

  out->file = NULL;
  out->file = fopen(filename, "wb");
  if (out->file == NULL) {
    set_error(ERR_MIP_NOFILE);
    MipCloseOutputFile(out);
    return NULL;
  }

  out->version = MIP_FILE_VERSION;
  out->width = 0;
  out->height = 0;
  out->nchannels = 0;
  out->tilesize = 0;
  out->fb = NULL;

  return out;
}

void MipCloseOutputFile(struct MipOutput *out)
{
  if (out == NULL)
    return;

  if (out->file != NULL) {
    fclose(out->file);
  }

  FbFree(out->fb);
  FJ_MEM_FREE(out);
}

int MipGenerateFromSourceData(struct MipOutput *out,
    const float *pixels, int width, int height, int nchannels)
{
  assert(pixels != NULL);

  if (nchannels != 1 && nchannels != 3) {
    return -1;
  }
  out->nchannels = nchannels;

  out->width = 0;
  out->height = 0;
  compute_output_res(width, height, &out->width, &out->height);
  if (out->width == 0 || out->height == 0) {
    return -1;
  }

  out->fb = FbNew();
  if (out->fb == NULL) {
    return -1;
  }
  out->fb->Resize(out->width, out->height, out->nchannels);
  scale_and_copy_image(pixels, width, height,
      out->fb->GetWritable(0, 0, 0), out->width, out->height, out->nchannels);

  out->tilesize = Min(64, out->width);
  out->tilesize = Min(out->tilesize, out->height);

  return 0;
}

void MipWriteFile(struct MipOutput *out)
{
  size_t nwrites;
  int x, y;
  int TILESIZE;
  int XNTILES, YNTILES;
  char magic[] = MIP_FILE_MAGIC;

  TILESIZE = out->tilesize;

  nwrites = 0;
  nwrites += sizeof(char) * fwrite(magic, sizeof(char), MIP_MAGIC_SIZE, out->file);
  nwrites += sizeof(int) *  fwrite(&out->version, sizeof(int), 1, out->file);
  nwrites += sizeof(int) *  fwrite(&out->width, sizeof(int), 1, out->file);
  nwrites += sizeof(int) *  fwrite(&out->height, sizeof(int), 1, out->file);
  nwrites += sizeof(int) *  fwrite(&out->nchannels, sizeof(int), 1, out->file);
  nwrites += sizeof(int) *  fwrite(&TILESIZE, sizeof(int), 1, out->file);

  XNTILES = out->width / TILESIZE;
  YNTILES = out->height / TILESIZE;

  for (y = 0; y < YNTILES; y++) {
    for (x = 0; x < XNTILES; x++) {
      int i;
      for (i = 0; i < TILESIZE; i++) {
        const float *line = out->fb->GetReadOnly(x * TILESIZE, y * TILESIZE + i, 0);
        fwrite(line, sizeof(float), TILESIZE * out->nchannels, out->file);
      }
    }
  }
}

static void set_error(int err)
{
  error_no = err;
}

static void compute_output_res(int in_w, int in_h, int *out_w, int *out_h)
{
  int i;

  for (i = 0; i < POW2_SIZE; i++) {
    if (in_w <= pow2[i]) {
      *out_w = pow2[i];
      break;
    }
  }
  for (i = 0; i < POW2_SIZE; i++) {
    if (in_h <= pow2[i]) {
      *out_h = pow2[i];
      break;
    }
  }
}

static void scale_and_copy_image(const float *src_pxls, int sw, int sh,
    float *dst_pxls, int dw, int dh, int nchannels)
{
  int dst_x, dst_y;
  int DST_W, DST_H, SRC_W, SRC_H;
  int NCHANS;
  float XSCALE, YSCALE;

  if (dw == sw && dh == sh) {
    memcpy(dst_pxls, src_pxls, sizeof(float) * sw * sh * nchannels);
    return;
  }

  DST_W = dw;
  DST_H = dh;
  SRC_W = sw;
  SRC_H = sh;
  NCHANS = nchannels;
  XSCALE = dw / (float) sw;
  YSCALE = dh / (float) sh;

  for (dst_y = 0; dst_y < DST_H; dst_y++) {
    struct Position src_pos;
    src_pos.x = 0;
    src_pos.y = (dst_y + .5) / YSCALE;

    for (dst_x = 0; dst_x < DST_W; dst_x++) {
      int i, ch;
      int src_idx[4];
      float weight[4], xweight0, xweight1, yweight0, yweight1;
      float *dst;
      struct Coordinate src_pix[4];
      struct Position ctr_pos;

      src_pos.x = (dst_x + .5) / XSCALE;
      ctr_pos.x = ROUND(src_pos.x);
      ctr_pos.y = ROUND(src_pos.y);

      src_pix[0].x = (int) ctr_pos.x - 1;
      src_pix[1].x = (int) ctr_pos.x;
      src_pix[2].x = (int) ctr_pos.x - 1;
      src_pix[3].x = (int) ctr_pos.x;
      src_pix[0].y = (int) ctr_pos.y - 1;
      src_pix[1].y = (int) ctr_pos.y - 1;
      src_pix[2].y = (int) ctr_pos.y;
      src_pix[3].y = (int) ctr_pos.y;
      for (i = 0; i < 4; i++) {
        src_pix[i].x = Clamp(src_pix[i].x, 0, SRC_W-1);
        src_pix[i].y = Clamp(src_pix[i].y, 0, SRC_H-1);
        src_idx[i] = src_pix[i].y * SRC_W * NCHANS + src_pix[i].x * NCHANS;
      }

      xweight0 = .5 - (src_pos.x - ctr_pos.x);
      xweight1 = 1 - xweight0;
      yweight0 = .5 - (src_pos.y - ctr_pos.y);
      yweight1 = 1 - yweight0;
      weight[0] = xweight0 * yweight0;
      weight[1] = xweight1 * yweight0;
      weight[2] = xweight0 * yweight1;
      weight[3] = xweight1 * yweight1;

      dst = dst_pxls + (dst_y * DST_W * NCHANS) + (dst_x * NCHANS);
      for (ch = 0; ch < NCHANS; ch++) {
        dst[ch] = 0;
        dst[ch] += src_pxls[src_idx[0]+ch] * weight[0];
        dst[ch] += src_pxls[src_idx[1]+ch] * weight[1];
        dst[ch] += src_pxls[src_idx[2]+ch] * weight[2];
        dst[ch] += src_pxls[src_idx[3]+ch] * weight[3];
      }
    }
  }
}

} // namespace xxx
