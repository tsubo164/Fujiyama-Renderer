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
    str_("N/A"), num_(0), id_(SI_BADID) {} //TODO string for invalid argument
  ~CommandArgument() {}

public:
  void SetString(const std::string &str)
  {
    str_ = str;
  }
  const char *GetString() const
  {
    return str_.c_str();
  }

  void SetNumber(double num)
  {
    num_ = num;
  }
  double GetNumber() const
  {
    return num_;
  }

  void SetID(ID id)
  {
    id_ = id;
  }
  ID GetID() const
  {
    return id_;
  }

private:
  std::string str_;
  double num_;
  ID id_;
};

class CommandResult {
public:
  CommandResult() :
      status_(SI_FAIL),
      new_id_(SI_BADID),
      entry_name_("")
  {}
  ~CommandResult() {}

public:
  void SetStatus(Status status)
  {
    status_ = status;
  }
  Status GetStatus() const
  {
    return status_;
  }
  void SetEntryID(ID id)
  {
    new_id_ = id;
  }
  ID GetEntryID() const
  {
    return new_id_;
  }
  void SetEntryName(const std::string &name)
  {
    entry_name_ = name;
  }
  const char *GetEntryName() const
  {
    return entry_name_.c_str();
  }

  bool HasEntryName() const
  {
    return !entry_name_.empty();
  }

  bool IsFail() const
  {
    return GetStatus() == SI_FAIL && GetEntryID() == SI_BADID;
  }

private:
  Status status_;
  ID new_id_;
  std::string entry_name_;
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

#endif // XXX_H
