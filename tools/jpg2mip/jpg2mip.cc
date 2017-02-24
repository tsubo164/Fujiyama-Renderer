// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_framebuffer.h"
#include "fj_mipmap.h"
#include "fj_box.h"

#include <jpeglib.h>
#include <setjmp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

using namespace fj;

static const char USAGE[] =
"Usage: jpg2mip [options] inputfile(*.jpeg, *.jpg) outputfile(*.mip)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

static void copy_scanline(JSAMPROW j_scanline, float *fb_scanline, int width, int nchans);

int main(int argc, const char **argv)
{
  int width = 0;
  int height = 0;
  int nchans = 0;
  MipOutput mip;
  FrameBuffer fb;
  int i;

  /* jpeg */
  FILE *jpg_file = NULL;
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  JSAMPARRAY buffer;
  int row_stride;

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    printf("%s", USAGE);
    return 0;
  }

  if (argc != 3) {
    fprintf(stderr, "error: invalid number of arguments.\n");
    fprintf(stderr, "%s", USAGE);
    return -1;
  }

  errno = 0;
  if ((jpg_file = fopen(argv[1], "rb")) == NULL) {
    fprintf(stderr, "error: %s: %s\n", argv[1], strerror(errno));
    return -1;
  }

  /* read jpeg header */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    goto cleanup_jpeg;
  }
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, jpg_file);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);

  width  = cinfo.output_width;
  height = cinfo.output_height;
  nchans = cinfo.output_components;

  if (width <= 0 || height <= 0) {
    fprintf(stderr, "error: invalid image size detected: %d x %d\n", width, height);
    goto cleanup_jpeg;
  }

  if (fb.Resize(width, height, nchans) == NULL) {
    fprintf(stderr, "error: could not allocate framebuffer: %d x %d\n", width, height);
    goto cleanup_jpeg;
  }

  /* allocate jpeg */
  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  /* read jpeg */
  i = 0;
  while (cinfo.output_scanline < cinfo.output_height) {
    float * fb_scanline = fb.GetWritable(0, i, 0);
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    copy_scanline(buffer[0], fb_scanline, width, nchans);
    i++;
  }

  /* finish decompression */
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(jpg_file);

  mip.Open(argv[2]);
  if (!mip.IsOpen()) {
    fprintf(stderr, "error: couldn't open output file\n");
    return -1;
  }

  mip.GenerateFromSourceData(fb.GetReadOnly(0, 0, 0), width, height, nchans);
  printf("input res: %d, %d\n", width, height);
  printf("output res: %d, %d\n", mip.GetWidth(), mip.GetHeight());

  mip.WriteFile();

  return 0;

cleanup_jpeg:
  jpeg_destroy_decompress(&cinfo);
  fclose(jpg_file);
  return -1;
}

static void copy_scanline(JSAMPROW j_scanline, float *fb_scanline, int width, int nchans)
{
  const int N = width * nchans;
  int i;

  for (i = 0; i < N; i++) {
    fb_scanline[i] = j_scanline[i] / 255.f;
  }
}
