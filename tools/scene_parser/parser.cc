// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "parser.h"

#include <sstream>
#include <vector>
#include <cassert>
#include <cstdio>
#include <cstdlib>

static void print_command(const CommandArgument *args, int nargs);
static int symbol_to_number(CommandArgument *arg);
static int scan_number(CommandArgument *arg);
static int args_from_tokens(const std::vector<std::string> &tokens,
    CommandArgument *args, int max_args);
static int tokenize(const std::string &str, std::vector<std::string> &tokens);

enum PsrErroNo {
  PSR_ERR_NONE = 1024, // offset to avoid conflict with SI_ERR
  PSR_ERR_UNKNOWN_COMMAND,
  PSR_ERR_MANY_ARGS,
  PSR_ERR_FEW_ARGS,
  PSR_ERR_BAD_NUMBER,
  PSR_ERR_BAD_ENUM,
  PSR_ERR_NAME_EXISTS,
  PSR_ERR_NAME_NOT_FOUND
};

Parser::Parser() :
  name_map_(),
  line_no_(0),
  error_message_(NULL),
  error_no_()
{
  SiOpenScene();
  parse_error(PSR_ERR_NONE);
}

Parser::~Parser()
{
  SiCloseScene();
}

int Parser::ParseLine(const std::string &line)
{
  std::vector<std::string> tokens;
  const int ntokens = tokenize(line, tokens);

  line_no_++;

  if (ntokens == 0) {
    // blank line
    return 0;
  }
  if (tokens[0][0] == '#') {
    // comment line
    return 0;
  }

  CommandArgument arguments[16];
  args_from_tokens(tokens, arguments, 16);

  const Command *command = SearchCommand(arguments[0].GetString());
  if (command == NULL) {
    parse_error(PSR_ERR_UNKNOWN_COMMAND);
    return -1;
  }
  if (ntokens < command->arg_count) {
    parse_error(PSR_ERR_FEW_ARGS);
    return -1;
  }
  if (ntokens > command->arg_count) {
    parse_error(PSR_ERR_MANY_ARGS);
    return -1;
  }

  const int err = build_arguments(command, arguments);
  if (err) {
    return -1;
  }

  print_command(arguments, command->arg_count);

  const CommandResult result = command->Run(arguments);
  if (result.IsFail()) {
    parse_error(SiGetErrorNo());
    return -1;
  }

  if (result.HasEntryName()) {
    register_name(result.GetEntryName(), result.GetEntryID());
  }

  return 0;
}

int Parser::GetLineNumber() const
{
  return line_no_;
}

const char *Parser::GetErrorMessage() const
{
  return error_message_;
}

bool Parser::register_name(const std::string &name, ID id)
{
  NameMap::const_iterator it = name_map_.find(name);
  if (it == name_map_.end()) {
    name_map_[name] = id;
    return true;
  } else {
    return false;
  }
}

ID Parser::lookup_name(const std::string &name) const
{
  NameMap::const_iterator it = name_map_.find(name);
  if (it != name_map_.end()) {
    return it->second;
  } else {
    return SI_BADID;
  }
}

int Parser::build_arguments(const Command *command, CommandArgument *arguments)
{
  for (int i = 0; i < command->arg_count; i++) {
    CommandArgument *arg = &arguments[i];
    const int type = command->arg_types[i];

    switch(type) {
    case ARG_NEW_ENTRY_ID:
      if (lookup_name(arg->GetString()) != SI_BADID) {
        parse_error(PSR_ERR_NAME_EXISTS);
        return -1;
      }
      break;

    case ARG_ENTRY_ID: {
      const ID id = lookup_name(arg->GetString());
      if (id == SI_BADID) {
        parse_error(PSR_ERR_NAME_NOT_FOUND);
        return -1;
      }
      arg->SetID(id);
      break;
      }

    case ARG_NUMBER: {
      const int err = scan_number(arg);
      if (err) {
        parse_error(PSR_ERR_BAD_NUMBER);
        return -1;
      }
      break;
      }

    case ARG_LIGHT_TYPE: {
      const std::string str = arg->GetString();
      if (str == "PointLight") {
        arg->SetNumber(SI_POINT_LIGHT);
      } else if (str == "GridLight") {
        arg->SetNumber(SI_GRID_LIGHT);
      } else if (str == "SphereLight") {
        arg->SetNumber(SI_SPHERE_LIGHT);
      } else if (str == "DomeLight") {
        arg->SetNumber(SI_DOME_LIGHT);
      } else {
        parse_error(PSR_ERR_BAD_ENUM);
        return -1;
      }
      }
      break;

    case ARG_PROPERTY_NAME:
      break;
    case ARG_GROUP_NAME: {
      const std::string str = arg->GetString();
      if (str == "DEFAULT_SHADING_GROUP") {
        arg->SetString("");
      }
      }
      break;
    case ARG_FILE_PATH:
      break;
    case ARG_STRING:
      break;
    case ARG_COMMAND_NAME:
      break;
    default:
      assert(!"implement error\n");
      break;
    }
  }
  return 0;
}
// TODO should move this to fj_scene_interface.c?
class SiError {
public:
  int number;
  const char *message;
};

void Parser::parse_error(int error_no)
{
  static const SiError errors[] = {
    // from Parser
    {PSR_ERR_NONE, ""},
    {PSR_ERR_UNKNOWN_COMMAND,  "unknown command"},
    {PSR_ERR_MANY_ARGS,        "too many arguments"},
    {PSR_ERR_FEW_ARGS,         "too few arguments"},
    {PSR_ERR_BAD_NUMBER,       "bad number arguments"},
    {PSR_ERR_BAD_ENUM,         "bad enum arguments"},
    {PSR_ERR_NAME_EXISTS,      "entry name already exists"},
    {PSR_ERR_NAME_NOT_FOUND,   "entry name not found"},
    // from SceneInterface
    {SI_ERR_PLUGIN_NOT_FOUND,           "plugin not found"},
    {SI_ERR_INIT_PLUGIN_FUNC_NOT_EXIST, "initialize plugin function not exit"},
    {SI_ERR_INIT_PLUGIN_FUNC_FAIL,      "initialize plugin function failed"},
    {SI_ERR_BAD_PLUGIN_INFO,            "invalid plugin info in the plugin"},
    {SI_ERR_CLOSE_PLUGIN_FAIL,          "close plugin function failed"},
    // TODO FIXME temp
    {SI_ERR_BADTYPE,    "invalid entry type"},
    {SI_ERR_FAILLOAD,   "load file failed"},
    {SI_ERR_FAILNEW,    "new entry failed"},
    {SI_ERR_NO_MEMORY,  "no memory"},
    {SI_ERR_NONE, ""}
  };

  error_no_ = error_no;

  for (const SiError *error = errors; error->number != SI_ERR_NONE; error++) {
    if (error_no_ == error->number) {
      error_message_ = error->message;
      break;
    }
  }
}
static int tokenize(const std::string &str, std::vector<std::string> &tokens)
{
  std::istringstream iss(str);
  std::string s;
  tokens.clear();

  while (!iss.eof()) {
    s.clear();

    iss >> s;
    if (!iss) {
      break;
    }

    tokens.push_back(s);
  }
  return static_cast<int>(tokens.size());
}

static int args_from_tokens(const std::vector<std::string> &tokens,
    CommandArgument *args, int max_args)
{
  const int ntokens = static_cast<int>(tokens.size());

  for (int i = 0; i < ntokens && i < max_args; i++) {
    args[i].SetString(tokens[i]);
  }

  return ntokens;
}

static void print_command(const CommandArgument *args, int nargs)
{
  printf("-- %s: ", args[0].GetString());
  for (int i = 1; i < nargs; i++) {
    printf("[%s]", args[i].GetString());
    if (i == nargs - 1) {
      printf("\n");
    } else {
      printf(" ");
    }
  }
}

static int scan_number(CommandArgument *arg)
{
  char *end = NULL;
  const int is_symbol = symbol_to_number(arg);

  if (is_symbol) {
    // number is already set from symbol string
    return 0;
  }

  const double n = strtod(arg->GetString(), &end);
  arg->SetNumber(n);
  if (*end != '\0') {
    return -1;
  }

  return 0;
}

static int symbol_to_number(CommandArgument *arg)
{
  const std::string str = arg->GetString();

  // transform orders
  if (str == "ORDER_SRT") {arg->SetNumber(SI_ORDER_SRT); return 1;}
  if (str == "ORDER_STR") {arg->SetNumber(SI_ORDER_STR); return 1;}
  if (str == "ORDER_RST") {arg->SetNumber(SI_ORDER_RST); return 1;}
  if (str == "ORDER_RTS") {arg->SetNumber(SI_ORDER_RTS); return 1;}
  if (str == "ORDER_TRS") {arg->SetNumber(SI_ORDER_TRS); return 1;}
  if (str == "ORDER_TSR") {arg->SetNumber(SI_ORDER_TSR); return 1;}
  // rotate orders
  if (str == "ORDER_XYZ") {arg->SetNumber(SI_ORDER_XYZ); return 1;}
  if (str == "ORDER_XZY") {arg->SetNumber(SI_ORDER_XZY); return 1;}
  if (str == "ORDER_YXZ") {arg->SetNumber(SI_ORDER_YXZ); return 1;}
  if (str == "ORDER_YZX") {arg->SetNumber(SI_ORDER_YZX); return 1;}
  if (str == "ORDER_ZXY") {arg->SetNumber(SI_ORDER_ZXY); return 1;}
  if (str == "ORDER_ZYX") {arg->SetNumber(SI_ORDER_ZYX); return 1;}

  // sampler type
  if (str == "FIXED_GRID_SAMPER")     {arg->SetNumber(SI_FIXED_GRID_SAMPLER); return 1;}
  if (str == "ADAPTIVE_GRID_SAMPLER") {arg->SetNumber(SI_ADAPTIVE_GRID_SAMPLER); return 1;}

  return 0;
}
