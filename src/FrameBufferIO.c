/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "FrameBufferIO.h"
#include "FrameBuffer.h"
#include "Memory.h"
#include "Vector.h"
#include "Box.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FB_FILE_VERSION 1
#define FB_FILE_MAGIC "FBUF"
#define FB_MAGIC_SIZE 4

static void set_error(int err);

static int error_no = ERR_FB_NOERR;

int FbGetErrorNo(void)
{
  return error_no;
}

const char *FbGetErrorMessage(int err)
{
  static const char *errmsg[] = {
    "",                      /* ERR_FB_NOERR */
    "No memory for FbInput", /* ERR_FB_NOMEM */
    "No such file",          /* ERR_FB_NOFILE */
    "Not framebuffer file",  /* ERR_FB_NOTFB */
    "Invalid file version"   /* ERR_FB_BADVER */
  };
  static const int nerrs = (int )sizeof(errmsg)/sizeof(errmsg[0]);

  if (err >= nerrs) {
    fprintf(stderr, "fatal error: error no %d is out of range\n", err);
    abort();
  }
  return errmsg[err];
}

struct FbInput *FbOpenInputFile(const char *filename)
{
  struct FbInput *in = MEM_ALLOC(struct FbInput);

  if (in == NULL) {
    set_error(ERR_FB_NOMEM);
    return NULL;
  }

  errno = 0;
  in->file = fopen(filename, "rb");
  if (in->file == NULL) {
    set_error(ERR_FB_NOFILE);
    MEM_FREE(in);
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

void FbCloseInputFile(struct FbInput *in)
{
  if (in == NULL)
    return;

  if (in->file != NULL) {
    fclose(in->file);
  }
  MEM_FREE(in);
}

int FbReadHeader(struct FbInput *in)
{
  size_t nreads = 0;
  char magic[FB_MAGIC_SIZE] = {'\0'};

  nreads += fread(magic, sizeof(char), FB_MAGIC_SIZE, in->file);
  if (memcmp(magic, FB_FILE_MAGIC, FB_MAGIC_SIZE) != 0) {
    set_error(ERR_FB_NOTFB);
    return -1;
  }
  nreads += fread(&in->version, sizeof(int), 1, in->file);
  if (in->version != FB_FILE_VERSION) {
    set_error(ERR_FB_BADVER);
    return -1;
  }
  nreads += fread(&in->width, sizeof(int), 1, in->file);
  nreads += fread(&in->height, sizeof(int), 1, in->file);
  nreads += fread(&in->nchannels, sizeof(int), 1, in->file);
  nreads += fread(in->viewbox, sizeof(int), 4, in->file);
  nreads += fread(in->databox, sizeof(int), 4, in->file);

  return 0;
}

int FbReadData(struct FbInput *in)
{
  size_t nreads = 0;

  if (in->data == NULL)
    return -1;

  nreads += fread(in->data, sizeof(float), in->width * in->height * in->nchannels, in->file);
  return 0;
}

struct FbOutput *FbOpenOutputFile(const char *filename)
{
  struct FbOutput *out = MEM_ALLOC(struct FbOutput);

  if (out == NULL) {
    set_error(ERR_FB_NOMEM);
    return NULL;
  }

  out->file = fopen(filename, "wb");
  if (out->file == NULL) {
    set_error(ERR_FB_NOFILE);
    MEM_FREE(out);
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

void FbCloseOutputFile(struct FbOutput *out)
{
  if (out == NULL)
    return;

  if (out->file != NULL) {
    fclose(out->file);
  }
  MEM_FREE(out);
}

void FbWriteFile(struct FbOutput *out)
{
  char magic[] = FB_FILE_MAGIC;

  fwrite(magic, sizeof(char), FB_MAGIC_SIZE, out->file);
  fwrite(&out->version, sizeof(int), 1, out->file);
  fwrite(&out->width, sizeof(int), 1, out->file);
  fwrite(&out->height, sizeof(int), 1, out->file);
  fwrite(&out->nchannels, sizeof(int), 1, out->file);
  fwrite(out->viewbox, sizeof(int), 4, out->file);
  fwrite(out->databox, sizeof(int), 4, out->file);
  fwrite(out->data, sizeof(float), out->width * out->height * out->nchannels, out->file);
}

int FbSaveCroppedData(struct FrameBuffer *fb, const char *filename)
{
  int x, y;
  int xmin, ymin, xmax, ymax;
  int viewbox[4] = {0, 0, 0, 0};
  int databox[4] = {0, 0, 0, 0};
  struct FrameBuffer *cropped = NULL;
  struct FbOutput *out = NULL;

  out = FbOpenOutputFile(filename);
  if (out == NULL) {
    return -1;
  }

  FbComputeBounds(fb, databox);

  xmin = databox[0];
  ymin = databox[1];
  xmax = databox[2];
  ymax = databox[3];
  BOX2_SET(viewbox, 0, 0, FbGetWidth(fb), FbGetHeight(fb));

  cropped = FbNew();
  FbResize(cropped, xmax-xmin, ymax-ymin, FbGetChannelCount(fb));

  for ( y = ymin; y < ymax; y++) {
    for ( x = xmin; x < xmax; x++) {
      const float *src = (float *) FbGetReadOnly(fb, x, y, 0);
      float *dst = FbGetWritable(cropped, x-xmin, y-ymin, 0);
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = src[3];
    }
  }

  out->width = FbGetWidth(cropped);
  out->height = FbGetHeight(cropped);
  out->nchannels = FbGetChannelCount(cropped);
  BOX2_COPY(out->viewbox, viewbox);
  BOX2_COPY(out->databox, databox);
  out->data = FbGetReadOnly(cropped, 0, 0, 0);

  FbWriteFile(out);
  FbCloseOutputFile(out);

  FbFree(cropped);

  return 0;
}

static void set_error(int err)
{
  error_no = err;
}

