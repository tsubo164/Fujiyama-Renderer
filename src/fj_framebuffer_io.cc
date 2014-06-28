// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_framebuffer_io.h"
#include "fj_framebuffer.h"
#include "fj_file_io.h"
#include "fj_vector.h"
#include "fj_box.h"

#include <cstdlib>
#include <cstring>
#include <cerrno>

#define FB_FILE_VERSION 1
#define FB_FILE_MAGIC "FBUF"
#define FB_MAGIC_SIZE 4

namespace fj {

static void set_error(int err);

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

FbInput *FbOpenInputFile(const char *filename)
{
  FbInput *in = new FbInput();

  if (in == NULL) {
    set_error(ERR_FB_NOMEM);
    return NULL;
  }

  errno = 0;
  in->file = fopen(filename, "rb");
  if (in->file == NULL) {
    set_error(ERR_FB_NOFILE);
    delete in;
    return NULL;
  }

  in->version = 0;
  in->width = 0;
  in->height = 0;
  in->nchannels = 0;
  BOX2_SET(in->viewbox, 0, 0, 0, 0);
  BOX2_SET(in->databox, 0, 0, 0, 0);
  in->data = NULL;

  return in;
}

void FbCloseInputFile(FbInput *in)
{
  if (in == NULL)
    return;

  if (in->file != NULL) {
    fclose(in->file);
  }
  delete in;
}

int FbReadHeader(FbInput *in)
{
  int8_t magic[FB_MAGIC_SIZE] = {'\0'};
  int err = 0;

  err = FjFile_ReadInt8(in->file, magic, FB_MAGIC_SIZE);
  if (err || memcmp(magic, FB_FILE_MAGIC, FB_MAGIC_SIZE) != 0) {
    set_error(ERR_FB_NOTFB);
    return -1;
  }
  err = FjFile_ReadInt32(in->file, &in->version, 1);
  if (err || in->version != FB_FILE_VERSION) {
    set_error(ERR_FB_BADVER);
    return -1;
  }

  err |= FjFile_ReadInt32(in->file, &in->width, 1);
  err |= FjFile_ReadInt32(in->file, &in->height, 1);
  err |= FjFile_ReadInt32(in->file, &in->nchannels, 1);
  err |= FjFile_ReadInt32(in->file, in->viewbox, 4);
  err |= FjFile_ReadInt32(in->file, in->databox, 4);
  if (err) {
    // TODO error handling
  }

  return 0;
}

int FbReadData(FbInput *in)
{
  int err = 0;

  if (in->data == NULL)
    return -1;

  err = FjFile_ReadFloat(in->file, in->data, in->width * in->height * in->nchannels);
  if (err) {
    // TODO error handling
  }
  return 0;
}

FbOutput *FbOpenOutputFile(const char *filename)
{
  FbOutput *out = new FbOutput();

  if (out == NULL) {
    set_error(ERR_FB_NOMEM);
    return NULL;
  }

  out->file = fopen(filename, "wb");
  if (out->file == NULL) {
    set_error(ERR_FB_NOFILE);
    delete out;
    return NULL;
  }

  out->version = FB_FILE_VERSION;
  out->width = 0;
  out->height = 0;
  out->nchannels = 0;
  BOX2_SET(out->viewbox, 0, 0, 0, 0);
  BOX2_SET(out->databox, 0, 0, 0, 0);
  out->data = NULL;

  return out;
}

void FbCloseOutputFile(FbOutput *out)
{
  if (out == NULL)
    return;

  if (out->file != NULL) {
    fclose(out->file);
  }
  delete out;
}

void FbWriteFile(FbOutput *out)
{
  int8_t magic[] = FB_FILE_MAGIC;
  int err = 0;

  err |= FjFile_WriteInt8 (out->file, magic, FB_MAGIC_SIZE);
  err |= FjFile_WriteInt32(out->file, &out->version, 1);
  err |= FjFile_WriteInt32(out->file, &out->width, 1);
  err |= FjFile_WriteInt32(out->file, &out->height, 1);
  err |= FjFile_WriteInt32(out->file, &out->nchannels, 1);
  err |= FjFile_WriteInt32(out->file, out->viewbox, 4);
  err |= FjFile_WriteInt32(out->file, out->databox, 4);
  err |= FjFile_WriteFloat(out->file, out->data, out->width * out->height * out->nchannels);
  if (err) {
    // TODO error handling
  }
}

int FbSaveCroppedData(FrameBuffer *fb, const char *filename)
{
  int x, y;
  int xmin, ymin, xmax, ymax;
  int viewbox[4] = {0, 0, 0, 0};
  int databox[4] = {0, 0, 0, 0};
  FbOutput *out = NULL;

  out = FbOpenOutputFile(filename);
  if (out == NULL) {
    return -1;
  }

  fb->ComputeBounds(databox);

  xmin = databox[0];
  ymin = databox[1];
  xmax = databox[2];
  ymax = databox[3];
  BOX2_SET(viewbox, 0, 0, fb->GetWidth(), fb->GetHeight());

  FrameBuffer cropped;
  cropped.Resize(xmax-xmin, ymax-ymin, fb->GetChannelCount());

  for ( y = ymin; y < ymax; y++) {
    for ( x = xmin; x < xmax; x++) {
      const float *src = (float *) fb->GetReadOnly(x, y, 0);
      float *dst = cropped.GetWritable(x-xmin, y-ymin, 0);
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = src[3];
    }
  }

  out->width = cropped.GetWidth();
  out->height = cropped.GetHeight();
  out->nchannels = cropped.GetChannelCount();
  BOX2_COPY(out->viewbox, viewbox);
  BOX2_COPY(out->databox, databox);
  out->data = cropped.GetReadOnly(0, 0, 0);

  FbWriteFile(out);
  FbCloseOutputFile(out);

  return 0;
}

static void set_error(int err)
{
  error_no = err;
}

} // namespace xxx
