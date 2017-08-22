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
  CommandArgument();
  ~CommandArgument();

public:
  void SetString(const std::string &str);
  const char *GetString() const;

  void SetNumber(double num);
  double GetNumber() const;

  void SetID(ID id);
  ID GetID() const;

private:
  std::string str_;
  double num_;
  ID id_;
};

class CommandResult {
public:
  CommandResult();
  ~CommandResult();

public:
  void SetStatus(Status status);
  Status GetStatus() const;

  void SetEntryID(ID id);
  ID GetEntryID() const;

  void SetEntryName(const std::string &name);
  const char *GetEntryName() const;

  bool HasEntryName() const;
  bool IsFail() const;

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

extern const Command *SearchCommand(const char *command_name);

#endif // XXX_H
