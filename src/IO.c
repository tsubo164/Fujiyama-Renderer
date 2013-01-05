/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "IO.h"
#include "String.h"
#include <string.h>

#define MAX_STR_SIZE 1024

int IOMatchMagic(FILE *file, const char *magic)
{
	size_t nreads;
	size_t magicsize;
	char filemagic[16] = {'\0'};

	magicsize = strlen(magic);
	if (magicsize > 16)
		return 0;

	nreads = fread(filemagic, sizeof(char), magicsize, file);
	if (nreads < magicsize)
		return 0;

	return memcmp(filemagic, magic, magicsize) != 0;
}

int IOMatchVersion(FILE *file, int version)
{
	int fileversion;
	size_t nreads;

	nreads = fread(&fileversion, sizeof(int), 1, file);
	if (nreads < 1)
		return 0;

	return version == fileversion;
}

size_t IOReadString(FILE *file, char *dst, size_t dstsize)
{
	size_t length;
	size_t nreads;

	length = 1;
	nreads = fread(&length, sizeof(size_t), 1, file);

	if (dstsize < length+1) {
		return 0;
	}

	nreads = fread(dst, sizeof(char), length, file);
	if (nreads < length) {
		return 0;
	}
	dst[length] = '\0';

	return nreads;
}

size_t IOReadInt(FILE *file, int *dst, size_t nelems)
{
	size_t nreads;

	nreads = fread(dst, sizeof(int), nelems, file);
	if (nreads < nelems) {
		return 0;
	}
	return nreads;
}

size_t IOReadFloat(FILE *file, float *dst, size_t nelems)
{
	size_t nreads;

	nreads = fread(dst, sizeof(float), nelems, file);
	if (nreads < nelems) {
		return 0;
	}
	return nreads;
}

size_t IOReadDouble(FILE *file, double *dst, size_t nelems)
{
	size_t nreads;

	nreads = fread(dst, sizeof(double), nelems, file);
	if (nreads < nelems) {
		return 0;
	}
	return nreads;
}

