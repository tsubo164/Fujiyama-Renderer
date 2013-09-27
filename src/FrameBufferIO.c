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

/* TODO TEST */
#include "Compatibility.h"
/* to get builtin types work with the macro */
typedef char char_type;
typedef float float_type;
typedef double double_type;
#define DEFINE_FILE_READ_WRITE(TYPE,SUFFIX) \
int file_read_##TYPE(FILE *file, TYPE##SUFFIX *dst, size_t count) \
{ \
  const size_t nreads = fread(dst, sizeof(*dst), count, file); \
  if (nreads != count) \
    return -1; \
  else \
    return 0; \
} \
int file_write_##TYPE(FILE *file, const TYPE##SUFFIX *src, size_t count) \
{ \
  const size_t nwrotes = fwrite(src, sizeof(*src), count, file); \
  if (nwrotes != count) \
    return -1; \
  else \
    return 0; \
}
DEFINE_FILE_READ_WRITE(char, _type)
DEFINE_FILE_READ_WRITE(int32, _t)
DEFINE_FILE_READ_WRITE(int64, _t)
DEFINE_FILE_READ_WRITE(float, _type)
DEFINE_FILE_READ_WRITE(double, _type)
/* TODO TEST */

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
  struct FbInput *in = SI_MEM_ALLOC(struct FbInput);

  if (in == NULL) {
    set_error(ERR_FB_NOMEM);
    return NULL;
  }

  errno = 0;
  in->file = fopen(filename, "rb");
  if (in->file == NULL) {
    set_error(ERR_FB_NOFILE);
    SI_MEM_FREE(in);
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
  SI_MEM_FREE(in);
}

int FbReadHeader(struct FbInput *in)
{
  char magic[FB_MAGIC_SIZE] = {'\0'};
  int err = 0;

  err = file_read_char(in->file, magic, FB_MAGIC_SIZE);
  if (err || memcmp(magic, FB_FILE_MAGIC, FB_MAGIC_SIZE) != 0) {
    set_error(ERR_FB_NOTFB);
    return -1;
  }
  err = file_read_int32(in->file, &in->version, 1);
  if (err || in->version != FB_FILE_VERSION) {
    set_error(ERR_FB_BADVER);
    return -1;
  }

  err |= file_read_int32(in->file, &in->width, 1);
  err |= file_read_int32(in->file, &in->height, 1);
  err |= file_read_int32(in->file, &in->nchannels, 1);
  err |= file_read_int32(in->file, in->viewbox, 4);
  err |= file_read_int32(in->file, in->databox, 4);
  /*
  FioReadInt32(in->file, in->databox, 4);
  */
  if (err) {
    /* TODO error handling */
  }

  return 0;
}

int FbReadData(struct FbInput *in)
{
  int err = 0;

  if (in->data == NULL)
    return -1;

  err = file_read_float(in->file, in->data, in->width * in->height * in->nchannels);
  if (err) {
    /* TODO error handling */
  }
  return 0;
}

struct FbOutput *FbOpenOutputFile(const char *filename)
{
  struct FbOutput *out = SI_MEM_ALLOC(struct FbOutput);

  if (out == NULL) {
    set_error(ERR_FB_NOMEM);
    return NULL;
  }

  out->file = fopen(filename, "wb");
  if (out->file == NULL) {
    set_error(ERR_FB_NOFILE);
    SI_MEM_FREE(out);
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
  SI_MEM_FREE(out);
}

void FbWriteFile(struct FbOutput *out)
{
  char magic[] = FB_FILE_MAGIC;
  int err = 0;

  err |= file_write_char(out->file, magic, FB_MAGIC_SIZE);
  err |= file_write_int32(out->file, &out->version, 1);
  err |= file_write_int32(out->file, &out->width, 1);
  err |= file_write_int32(out->file, &out->height, 1);
  err |= file_write_int32(out->file, &out->nchannels, 1);
  err |= file_write_int32(out->file, out->viewbox, 4);
  err |= file_write_int32(out->file, out->databox, 4);
  err |= file_write_float(out->file, out->data, out->width * out->height * out->nchannels);
  if (err) {
    /* TODO error handling */
  }
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
