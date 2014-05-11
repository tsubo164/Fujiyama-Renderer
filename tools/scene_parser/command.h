/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef COMMAND_H
#define COMMAND_H

#include "fj_scene_interface.h"

using namespace fj;

#ifdef __cplusplus
extern "C" {
#endif

enum ArgumentType {
  ARG_NULL = 0,
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_STRING,
  ARG_NUMBER,
  ARG_FILE_PATH,
  ARG_LIGHT_TYPE,
  ARG_UNDEFINED
};

struct CommandArgument {
  char *str;
  double num;
  ID id;
};

struct CommandResult {
  Status status;
  ID new_entry_id;
  const char *new_entry_name;
};
#define INIT_COMMAND_RESULT {SI_FAIL, SI_BADID, NULL}

typedef struct CommandResult (*CommandFunction)(const struct CommandArgument *args);

struct Command {
  const char *name;
  const int *arg_types;
  int arg_count;
  CommandFunction Run;
};

extern const struct Command *CmdSearchCommand(const char *command_name);
extern int CmdSuccess(const struct CommandResult *result);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

