// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "parser.h"
#include "fj_scene_interface.h"

#include <iostream>
#include <fstream>
#include <string>

#include <cstdio>
#include <cstring>
#include <cerrno>

const char *usage = "usage: scene [path]";

int main(int argc, const char **argv)
{
  std::istream *strm = NULL;
  std::ifstream file;
  std::string line;

  if (argc == 2) {
    file.open(argv[1]);
    if (!file) {
      fprintf(stderr, "error: %s: %s\n", argv[1], strerror(errno));
      return -1;
    }
    strm = &file;
  }
  else if (argc == 1) {
    strm = &std::cin;
  }
  else {
    fprintf(stderr, "%s\n", usage);
    return 0;
  }

  Parser parser;

  while (getline(*strm, line)) {
    const int err = PsrParseLine(&parser, line);

    if (err) {
      fprintf(stderr, "error: %s: %d: %s\n",
          PsrGetErrorMessage(&parser), PsrGetLineNo(&parser), line.c_str());
      return -1;
    }
  }

  return 0;
}
