/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "FrameBuffer.h"
#include "Mipmap.h"
#include "Box.h"
#include "rgbe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const char USAGE[] =
"Usage: hdr2mip [options] inputfile(*.hdr, *.rgbe) outputfile(*.mip)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

int main(int argc, const char **argv)
{
	FILE *fp;
	int width, height;
	struct MipOutput *mip;
	struct FrameBuffer *hdr;
	rgbe_header_info info;

	if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		printf(USAGE);
		return 0;
	}

	if (argc != 3) {
		fprintf(stderr, "error: invalid number of arguments.\n");
		fprintf(stderr, USAGE);
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

	hdr = FbNew();
	if (hdr == NULL) {
		fprintf(stderr, "error: could not allocate framebuffer itself\n");
		return -1;
	}

	if (FbResize(hdr, width, height, 3) == NULL) {
		fprintf(stderr, "error: could not allocate framebuffer: %d x %d\n", width, height);
		return -1;
	}
	RGBE_ReadPixels_RLE(fp, FbGetWritable(hdr, 0, 0, 0), width, height);

	if ((mip = MipOpenOutputFile(argv[2])) == NULL) {
		fprintf(stderr, "error: couldn't open output file\n");
		FbFree(hdr);
		return -1;
	}

	MipGenerateFromSourceData(mip, FbGetReadOnly(hdr, 0, 0, 0), width, height, 3);
	printf("input res: %d, %d\n", width, height);
	printf("output res: %d, %d\n", mip->width, mip->height);

	MipWriteFile(mip);

	MipCloseOutputFile(mip);
	FbFree(hdr);
	fclose(fp);

	return 0;
}

