// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef PARSER_H
#define PARSER_H

#include "fj_scene_interface.h"
#include <string>
#include <map>

using fj::ID;

class Parser {
public:
  Parser();
  ~Parser();

public:
  int line_no;

  const char *error_message;
  int error_no;

public:
  bool RegisterName(std::string name, ID id);
  ID LookupName(std::string name) const;

private:
  typedef std::map<std::string, ID> NameMap;
  NameMap name_map_;
};

extern Parser *PsrNew(void);
extern void PsrFree(Parser *parser);

//TODO TEST
extern int PsrParseLine(Parser *parser, const std::string &line);
extern int PsrGetLineNo(const Parser *parser);
extern const char *PsrGetErrorMessage(const Parser *parser);

#endif // FJ_XXX_H
