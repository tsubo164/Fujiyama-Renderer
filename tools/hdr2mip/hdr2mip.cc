// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_framebuffer.h"
#include "fj_mipmap.h"
#include "fj_box.h"
#include "rgbe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

using namespace fj;

static const char USAGE[] =
"Usage: hdr2mip [options] inputfile(*.hdr, *.rgbe) outputfile(*.mip)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

int main(int argc, const char **argv)
{
  FILE *fp;
  int width, height;
  MipOutput mip;
  FrameBuffer hdr;
  rgbe_header_info info;

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
  if ((fp = fopen(argv[1], "rb")) == NULL) {
    fprintf(stderr, "error: %s: %s\n", argv[1], strerror(errno));
    return -1;
  }

  RGBE_ReadHeader(fp, &width, &height, &info);
  if (width <= 0 || height <= 0) {
    fprintf(stderr, "error: invalid image size detected: %d x %d\n", width, height);
    return -1;
  }

  hdr.Resize(width, height, 3);
  if (hdr.IsEmpty()) {
    fprintf(stderr, "error: could not allocate framebuffer: %d x %d\n", width, height);
    return -1;
  }
  RGBE_ReadPixels_RLE(fp, hdr.GetWritable(0, 0, 0), width, height);

  mip.Open(argv[2]);
  if (!mip.IsOpen()) {
    fprintf(stderr, "error: couldn't open output file\n");
    return -1;
  }

  mip.GenerateFromSourceData(hdr.GetReadOnly(0, 0, 0), width, height, 3);
  printf("input res: %d, %d\n", width, height);
  printf("output res: %d, %d\n", mip.GetWidth(), mip.GetHeight());

  mip.WriteFile();

  fclose(fp);

  return 0;
}

