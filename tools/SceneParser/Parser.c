/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Parser.h"
#include "SceneInterface.h"
#include "Command.h"
#include "Table.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

enum PsrErroNo {
	PSR_ERR_NONE = 1024, /* offset to avoid conflict with SI_ERR */
	PSR_ERR_UNKNOWN_COMMAND,
	PSR_ERR_MANY_ARGS,
	PSR_ERR_FEW_ARGS,
	PSR_ERR_BAD_NUMBER,
	PSR_ERR_NAME_EXISTS,
	PSR_ERR_NAME_NOT_FOUND
};

struct Parser {
	int line_no;
	struct Table *table;

	const char *error_message;
	int error_no;
};

static void print_command(const struct CommandArgument *args, int nargs);
static int enum_to_num(struct CommandArgument *arg);
static int scan_number(struct CommandArgument *arg);
static int build_arguments(struct Parser *parser,
		const struct Command *command, struct CommandArgument *arguments);
static int tokenize_line(const char *line,
		char *tokenized, size_t tokenized_size,
		struct CommandArgument *args, int max_args);

static int parse_line(struct Parser *parser, const char *line);
static void parse_error(struct Parser *parser, int error_no);

struct Parser *PsrNew(void)
{
	struct Parser *parser;

	parser = (struct Parser *) malloc(sizeof(struct Parser));
	if (parser == NULL)
		return NULL;

	SiOpenScene();
	parser->line_no = 0;
	parser->table = TblNew();
	if (parser->table == NULL) {
		PsrFree(parser);
		return NULL;
	}

	parse_error(parser, PSR_ERR_NONE);

	return parser;
}

void PsrFree(struct Parser *parser)
{
	if (parser == NULL)
		return;

	SiCloseScene();

	TblFree(parser->table);
	free(parser);
}

const char *PsrGetErrorMessage(const struct Parser *parser)
{
	return parser->error_message;
}

int PsrParseLine(struct Parser *parser, const char *line)
{
	parser->line_no++;

	return parse_line(parser, line);
}

int PsrGetLineNo(const struct Parser *parser)
{
	return parser->line_no;
}

static int tokenize_line(const char *line,
		char *tokenized, size_t tokenized_size,
		struct CommandArgument *args, int max_args)
{
	struct CommandArgument *arg = args;
	const char *src = line;
	char *dst = tokenized;
	int ntokens = 0;

	while (*src != '\0' && *src != '\n' && ntokens < max_args) {
		arg->str = dst;

		while (*src != '\0' && isspace(*src)) {
			src++;
		}
		while (*src != '\0' && !isspace(*src)) {
			*dst++ = *src++;
		}
		*dst = '\0';
		dst++;
		arg++;
		ntokens++;
	}

	return ntokens;
}

static void print_command(const struct CommandArgument *args, int nargs)
{
	int i = 0;

	printf("-- %s: ", args[0].str);
	for (i = 1; i < nargs; i++) {
		printf("[%s]", args[i].str);
		if (i == nargs - 1) {
			printf("\n");
		} else {
			printf(" ");
		}
	}
}

static int build_arguments(struct Parser *parser,
		const struct Command *command, struct CommandArgument *arguments)
{
	int i;

	for (i = 0; i < command->arg_count; i++) {
		struct CommandArgument *arg = &arguments[i];
		const int *type = &command->arg_types[i];

		switch(*type) {
		case ARG_NEW_ENTRY_ID:
			if (TblLookup(parser->table, arg->str)) {
				parse_error(parser, PSR_ERR_NAME_EXISTS);
				return -1;
			}
			break;

		case ARG_ENTRY_ID: {
			const struct TableEnt *ent = TblLookup(parser->table, arg->str);
			if (ent == NULL) {
				parse_error(parser, PSR_ERR_NAME_NOT_FOUND);
				return -1;
			}
			arg->id = EntGetID(ent);
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

		case ARG_PROPERTY_NAME:
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

static int parse_line(struct Parser *parser, const char *line)
{
	struct CommandArgument arguments[16] = {{NULL}};
	struct CommandResult result = INIT_COMMAND_RESULT;
	const struct Command *command = NULL;

	const char *head = line;
	char token_list[1024] = {'\0'};
	int ntokens = 0;
	int err = 0;

	while (*head != '\0' && isspace(*head)) {
		head++;
	}

	if (head[0] == '\0') {
		return 0;
	}
	if (head[0] == '#') {
		return 0;
	}

	ntokens = tokenize_line(head, token_list, 1024, arguments, 16);
	command = CmdSearchCommand(arguments[0].str);
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

	err = build_arguments(parser, command, arguments);
	if (err) {
		return -1;
	}

	print_command(arguments, command->arg_count);

	result = command->Run(arguments);
	if (result.status == SI_FAIL) {
		parse_error(parser, SiGetErrorNo());
		return -1;
	}

	if (result.new_entry_name != NULL) {
		TblAdd(parser->table, result.new_entry_name, result.new_entry_id);
	}

	return 0;
}

static int scan_number(struct CommandArgument *arg)
{
	char *end = NULL;
	const int is_enum = enum_to_num(arg);

	if (is_enum)
		return 0;

	arg->num = strtod(arg->str, &end);
	if (*end != '\0') {
		return -1;
	}

	return 0;
}

static int enum_to_num(struct CommandArgument *arg)
{
	const char *str = arg->str;

	/* transform orders */
	if (strcmp(str, "ORDER_SRT") == 0) {arg->num = SI_ORDER_SRT; return 1;}
	if (strcmp(str, "ORDER_STR") == 0) {arg->num = SI_ORDER_STR; return 1;}
	if (strcmp(str, "ORDER_RST") == 0) {arg->num = SI_ORDER_RST; return 1;}
	if (strcmp(str, "ORDER_RTS") == 0) {arg->num = SI_ORDER_RTS; return 1;}
	if (strcmp(str, "ORDER_TRS") == 0) {arg->num = SI_ORDER_TRS; return 1;}
	if (strcmp(str, "ORDER_TSR") == 0) {arg->num = SI_ORDER_TSR; return 1;}
	/* rotate orders */
	if (strcmp(str, "ORDER_XYZ") == 0) {arg->num = SI_ORDER_XYZ; return 1;}
	if (strcmp(str, "ORDER_XZY") == 0) {arg->num = SI_ORDER_XZY; return 1;}
	if (strcmp(str, "ORDER_YXZ") == 0) {arg->num = SI_ORDER_YXZ; return 1;}
	if (strcmp(str, "ORDER_YZX") == 0) {arg->num = SI_ORDER_YZX; return 1;}
	if (strcmp(str, "ORDER_ZXY") == 0) {arg->num = SI_ORDER_ZXY; return 1;}
	if (strcmp(str, "ORDER_ZYX") == 0) {arg->num = SI_ORDER_ZYX; return 1;}

	return 0;
}

/* TODO should move this to SceneInterface.c? */
struct SiError {
	int number;
	const char *message;
};

static void parse_error(struct Parser *parser, int error_no)
{
	static const struct SiError errors[] = {
		/* from Parser */
		{PSR_ERR_NONE, ""},
		{PSR_ERR_UNKNOWN_COMMAND,  "unknown command"},
		{PSR_ERR_MANY_ARGS,        "too many arguments"},
		{PSR_ERR_FEW_ARGS,         "too few arguments"},
		{PSR_ERR_BAD_NUMBER,       "bad number arguments"},
		{PSR_ERR_NAME_EXISTS,      "entry name already exists"},
		{PSR_ERR_NAME_NOT_FOUND,   "entry name not found"},
		/* from SceneInterface */
		{SI_ERR_PLUGIN_NOT_FOUND,           "plugin not found"},
		{SI_ERR_INIT_PLUGIN_FUNC_NOT_EXIST, "initialize plugin function not exit"},
		{SI_ERR_INIT_PLUGIN_FUNC_FAIL,      "initialize plugin function failed"},
		{SI_ERR_BAD_PLUGIN_INFO,            "invalid plugin info in the plugin"},
		{SI_ERR_CLOSE_PLUGIN_FAIL,          "close plugin function failed"},
		{SI_ERR_NONE, ""}
	};
	const struct SiError *error = NULL;

	parser->error_no = error_no;

	for (error = errors; error->number != SI_ERR_NONE; error++) {
		if (parser->error_no == error->number) {
			parser->error_message = error->message;
			break;
		}
	}
}

