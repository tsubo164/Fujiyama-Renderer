// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "parser.h"
#include "fj_scene_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <float.h>

int main(int argc, const char **argv)
{
  Parser *parser = NULL;
  char buf[1024] = {'\0'};
  FILE *file = NULL;
  int status = 0;

  if (argc == 2) {
    file = fopen(argv[1], "r");
    if (file == NULL) {
      fprintf(stderr, "error: %s: %s\n", argv[1], strerror(errno));
      status = -1;
      goto cleanup_and_exit;
    }
  }
  else if (argc == 1) {
    file = stdin;
  }
  else {
    fprintf(stderr, "error: invalid number of arguments\n");
    status = -1;
    goto cleanup_and_exit;
  }

  parser = PsrNew();
  if (parser == NULL) {
    fprintf(stderr, "error: could not allocate a parser\n");
    status = -1;
    goto cleanup_and_exit;
  }

  while (fgets(buf, 1000, file) != NULL) {
    const int err = PsrParseLine(parser, buf);

    if (err) {
      fprintf(stderr, "error: %s: %d: %s",
          PsrGetErrorMessage(parser), PsrGetLineNo(parser), buf);
      status = -1;
      goto cleanup_and_exit;
    }
  }

cleanup_and_exit:
  if (file != stdin && file != NULL) {
    fclose(file);
  }
  PsrFree(parser);

  return status;
}
