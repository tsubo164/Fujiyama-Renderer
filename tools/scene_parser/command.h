// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef COMMAND_H
#define COMMAND_H

#include "fj_scene_interface.h"
#include <string>

using namespace fj;

enum ArgumentType {
  ARG_NULL = 0,
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_GROUP_NAME,
  ARG_STRING,
  ARG_NUMBER,
  ARG_FILE_PATH,
  ARG_LIGHT_TYPE,
  ARG_UNDEFINED
};

class CommandArgument {
public:
  CommandArgument() :
      str_(""),
      num(0),
      id(SI_BADID) {}
  ~CommandArgument() {}

public:
  void SetString(const std::string str)
  {
    str_ = str;
  }
  const char *AsString() const
  {
    if (str_ == "") {
      return "N/A"; //TODO string for invalid argument
    } else {
      return str_.c_str();
    }
  }

private:
  std::string str_;
public:
  //const char *str;
  double num;
  ID id;
};

class CommandResult {
public:
  CommandResult() :
      status(SI_FAIL),
      new_entry_id(SI_BADID),
      new_entry_name(NULL)
  {}
  ~CommandResult() {}

public:
  Status status;
  ID new_entry_id;
  const char *new_entry_name;
};

typedef CommandResult (*CommandFunction)(const CommandArgument *args);

class Command {
public:
  const char *name;
  const int *arg_types;
  int arg_count;
  CommandFunction Run;
};

extern const Command *CmdSearchCommand(const char *command_name);
extern int CmdSuccess(const CommandResult *result);

#endif // XXX_H
