// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef PARSER_H
#define PARSER_H

#include "fj_scene_interface.h"
#include "command.h"
#include <string>
#include <map>

using fj::ID;

class Parser {
public:
  Parser();
  ~Parser();

  int ParseLine(const std::string &line);
  int GetLineNumber() const;
  const char *GetErrorMessage() const;

private:
  typedef std::map<std::string, ID> NameMap;
  NameMap name_map_;
  int line_no_;
  const char *error_message_;
  int error_no_;

  bool register_name(const std::string &name, ID id);
  ID lookup_name(const std::string &name) const;
  int build_arguments(const Command *command, CommandArgument *arguments);
  void parse_error(int error_no);
};

#endif // FJ_XXX_H
