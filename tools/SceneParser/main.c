/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "SceneInterfaces.h"
#include "Parser.h"
#include "Vector.h"
#include "Box.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <float.h>


int main(int argc, const char **argv)
{
	char buf[1024] = {'\0'};
	int read_from_file = 0;
	struct Parser *parser;
	FILE *file;

	if (argc == 2) {
		file = fopen(argv[1], "r");
		if (file == NULL) {
			fprintf(stderr, "error: %s: %s\n", argv[1], strerror(errno));
			return -1;
		}
		read_from_file = 1;
	}
	else if (argc == 1) {
		file = stdin;
	}
	else {
		fprintf(stderr, "error: invalid number of arguments\n");
		return -1;
	}

	parser = PsrNew();
	if (parser == NULL) {
		fprintf(stderr, "error: could not allocate a parser\n");
		if (read_from_file) {
			fclose(file);
		}
		return -1;
	}

	while (fgets(buf, 1000, file) != NULL) {
		int err;

		err = PsrParseLine(parser, buf);
		if (err) {
			fprintf(stderr, "error: %s: %d: %s",
					PsrGetErrorMessage(PsrGetErrorNo()), PsrGetLineNo(parser), buf);
			return -1;
		}
	}

	if (read_from_file) {
		fclose(file);
	}
	PsrFree(parser);

	return 0;
}

