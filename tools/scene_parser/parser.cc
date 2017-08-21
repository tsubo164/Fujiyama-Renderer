// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "parser.h"
#include "command.h"

#include <sstream>
#include <vector>

#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <cctype>

static void print_command(const CommandArgument *args, int nargs);
static int symbol_to_number(CommandArgument *arg);
static int scan_number(CommandArgument *arg);
static int build_arguments(Parser *parser,
    const Command *command, CommandArgument *arguments);
static int args_from_tokens(const std::vector<std::string> &tokens,
    CommandArgument *args, int max_args);
static int tokenize(const std::string &str, std::vector<std::string> &tokens);

static int parse_line(Parser *parser, const std::string &line);
static void parse_error(Parser *parser, int error_no);

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

Parser::Parser()
{
  line_no = 0;
  SiOpenScene();
  parse_error(this, PSR_ERR_NONE);
}

Parser::~Parser()
{
  SiCloseScene();
}

bool Parser::RegisterName(std::string name, ID id)
{
  NameMap::const_iterator it = name_map_.find(name);
  if (it == name_map_.end()) {
    name_map_[name] = id;
    return true;
  } else {
    return false;
  }
}

ID Parser::LookupName(std::string name) const
{
  NameMap::const_iterator it = name_map_.find(name);
  if (it != name_map_.end()) {
    return it->second;
  } else {
    return SI_BADID;
  }
}

Parser *PsrNew(void)
{
  return new Parser();
}

void PsrFree(Parser *parser)
{
  delete parser;
}

const char *PsrGetErrorMessage(const Parser *parser)
{
  return parser->error_message;
}

int PsrParseLine(Parser *parser, const std::string &line)
{
  parser->line_no++;

  return parse_line(parser, line);
}

int PsrGetLineNo(const Parser *parser)
{
  return parser->line_no;
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
  int ntokens = static_cast<int>(tokens.size());

  for (int i = 0; i < ntokens && i < max_args; i++) {
    std::cout << "\t[" << tokens[i] << "]\n";
    args[i].SetString(tokens[i]);
  }

  return ntokens;
}

static void print_command(const CommandArgument *args, int nargs)
{
  int i = 0;

  printf("-- %s: ", args[0].AsString());
  for (i = 1; i < nargs; i++) {
    printf("[%s]", args[i].AsString());
    if (i == nargs - 1) {
      printf("\n");
    } else {
      printf(" ");
    }
  }
}

static int build_arguments(Parser *parser,
    const Command *command, CommandArgument *arguments)
{
  int i;

  for (i = 0; i < command->arg_count; i++) {
    CommandArgument *arg = &arguments[i];
    const int type = command->arg_types[i];

    switch(type) {
    case ARG_NEW_ENTRY_ID:
      if (parser->LookupName(arg->AsString()) != SI_BADID) {
        parse_error(parser, PSR_ERR_NAME_EXISTS);
        return -1;
      }
      break;

    case ARG_ENTRY_ID: {
      const ID id = parser->LookupName(arg->AsString());
      if (id == SI_BADID) {
        parse_error(parser, PSR_ERR_NAME_NOT_FOUND);
        return -1;
      }
      arg->id = id;
      break;
      }

    case ARG_NUMBER: {
      const int err = scan_number(arg);
      if (err) {
        parse_error(parser, PSR_ERR_BAD_NUMBER);
        return -1;
      }
      break;
      }

    case ARG_LIGHT_TYPE:
      if (strcmp(arg->AsString(), "PointLight") == 0) {
        arg->num = SI_POINT_LIGHT;
      } else if (strcmp(arg->AsString(), "GridLight") == 0) {
        arg->num = SI_GRID_LIGHT;
      } else if (strcmp(arg->AsString(), "SphereLight") == 0) {
        arg->num = SI_SPHERE_LIGHT;
      } else if (strcmp(arg->AsString(), "DomeLight") == 0) {
        arg->num = SI_DOME_LIGHT;
      } else {
        parse_error(parser, PSR_ERR_BAD_ENUM);
        return -1;
      }
      break;

    case ARG_PROPERTY_NAME:
      break;
    case ARG_GROUP_NAME:
      if (strcmp(arg->AsString(), "DEFAULT_SHADING_GROUP") == 0) {
        arg->SetString("");
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

static int parse_line(Parser *parser, const std::string &line)
{
  std::vector<std::string> tokens;
  const int ntokens = tokenize(line, tokens);

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

  const Command *command = CmdSearchCommand(arguments[0].AsString());
  if (command == NULL) {
    parse_error(parser, PSR_ERR_UNKNOWN_COMMAND);
    return -1;
  }
  if (ntokens < command->arg_count) {
    parse_error(parser, PSR_ERR_FEW_ARGS);
    return -1;
  }
  if (ntokens > command->arg_count) {
    parse_error(parser, PSR_ERR_MANY_ARGS);
    return -1;
  }

  const int err = build_arguments(parser, command, arguments);
  if (err) {
    return -1;
  }

  print_command(arguments, command->arg_count);

  const CommandResult result = command->Run(arguments);
  if (!CmdSuccess(&result)) {
    parse_error(parser, SiGetErrorNo());
    return -1;
  }

  if (result.new_entry_name != NULL) {
    parser->RegisterName(result.new_entry_name, result.new_entry_id);
  }

  return 0;
}

static int scan_number(CommandArgument *arg)
{
  char *end = NULL;
  const int is_symbol = symbol_to_number(arg);

  if (is_symbol) {
    // arg->num is already set from symbol string
    return 0;
  }

  arg->num = strtod(arg->AsString(), &end);
  if (*end != '\0') {
    return -1;
  }

  return 0;
}

static int symbol_to_number(CommandArgument *arg)
{
  const char *str = arg->AsString();

  // transform orders
  if (strcmp(str, "ORDER_SRT") == 0) {arg->num = SI_ORDER_SRT; return 1;}
  if (strcmp(str, "ORDER_STR") == 0) {arg->num = SI_ORDER_STR; return 1;}
  if (strcmp(str, "ORDER_RST") == 0) {arg->num = SI_ORDER_RST; return 1;}
  if (strcmp(str, "ORDER_RTS") == 0) {arg->num = SI_ORDER_RTS; return 1;}
  if (strcmp(str, "ORDER_TRS") == 0) {arg->num = SI_ORDER_TRS; return 1;}
  if (strcmp(str, "ORDER_TSR") == 0) {arg->num = SI_ORDER_TSR; return 1;}
  // rotate orders
  if (strcmp(str, "ORDER_XYZ") == 0) {arg->num = SI_ORDER_XYZ; return 1;}
  if (strcmp(str, "ORDER_XZY") == 0) {arg->num = SI_ORDER_XZY; return 1;}
  if (strcmp(str, "ORDER_YXZ") == 0) {arg->num = SI_ORDER_YXZ; return 1;}
  if (strcmp(str, "ORDER_YZX") == 0) {arg->num = SI_ORDER_YZX; return 1;}
  if (strcmp(str, "ORDER_ZXY") == 0) {arg->num = SI_ORDER_ZXY; return 1;}
  if (strcmp(str, "ORDER_ZYX") == 0) {arg->num = SI_ORDER_ZYX; return 1;}

  // sampler type
  if (strcmp(str, "FIXED_GRID_SAMPER") == 0)     {arg->num = SI_FIXED_GRID_SAMPLER; return 1;}
  if (strcmp(str, "ADAPTIVE_GRID_SAMPLER") == 0) {arg->num = SI_ADAPTIVE_GRID_SAMPLER; return 1;}

  return 0;
}

// TODO should move this to fj_scene_interface.c?
class SiError {
public:
  int number;
  const char *message;
};

static void parse_error(Parser *parser, int error_no)
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
  const SiError *error = NULL;

  parser->error_no = error_no;

  for (error = errors; error->number != SI_ERR_NONE; error++) {
    if (parser->error_no == error->number) {
      parser->error_message = error->message;
      break;
    }
  }
}
