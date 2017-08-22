// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "parser.h"
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, const char **argv)
{
  std::istream *strm = NULL;
  std::ifstream file;
  std::string line;

  switch (argc) {
  case 2:
    file.open(argv[1]);
    if (!file) {
      std::cerr << "error: Could not open file: " << argv[1] << std::endl;
      return -1;
    }
    strm = &file;
    break;
  case 1:
    strm = &std::cin;
    break;
  default:
    std::cerr << "usage: scene [path]" << std::endl;
    return 0;
  }

  Parser parser;

  while (getline(*strm, line)) {
    const int err = parser.ParseLine(line);

    if (err) {
      std::cerr << "error: ";
      std::cerr << parser.GetErrorMessage() << ": ";
      std::cerr << parser.GetLineNumber() << ": ";
      std::cerr << line.c_str() << std::endl;
      return -1;
    }
  }

  return 0;
}
