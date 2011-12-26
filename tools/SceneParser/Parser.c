/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Parser.h"
#include "Table.h"
#include "SceneInterfaces.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#define PROMPT "-- "

enum { MAX_ARGS = 6 };

union Argument {
	char str[1024];
	double dbl;
};

struct Parser {
	int line_no;
	struct Table *table;
};

static int parser_errno = ERR_PSR_NOERR;
static int run_command(struct Parser *parser, const char *cmd, const char *arg);
static int parse_args(const char *fmt, const char *arg, union Argument *args, int max_args);
static void set_errno(int err);
#if 0
static char error_detail[1024] = {'\0'};
static void set_error_detail(const char *detail);
static void append_error_detail(const char *detail);
#endif

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

static void set_errno(int err)
{
	parser_errno = err;
}

int PsrGetErrorNo(void)
{
	return parser_errno;
}

const char *PsrGetErrorMessage(int err_no)
{
	static const char *errmsg[] = {
		"",                    /* ERR_PSR_NOERR */
		"unknown command",     /* ERR_PSR_UNKNOWNCMD */
		"too many arguments",  /* ERR_PSR_MANYARGS */
		"too few arguments",   /* ERR_PSR_FEWARGS */
		"name already exists", /* ERR_PSR_NAMEEXISTS */
		"name not found",      /* ERR_PSR_NAMENOTFOUND */
		"plugin not found",    /* ERR_PSR_PLUGINNOTFOUND */
		"new entry failed",    /* ERR_PSR_FAILNEW */
		"render faided"        /* ERR_PSR_FAILRENDER */
	};
	static const size_t nerrs = sizeof(errmsg)/sizeof(errmsg[0]);

	if (err_no >= nerrs) {
		fprintf(stderr, "Logic error: err_no %d is out of range\n", err_no);
		abort();
	}
	return errmsg[err_no];
}

#if 0
const char *PsrGetErrorDetail(void)
{
	return error_detail;
}
#endif

int PsrParseLine(struct Parser *parser, const char *line)
{
	char trimed[1024] = {'\0'};
	char cmd[64] = {'\0'};
	int nreads;

	parser->line_no++;

	if (line[0] == '\n')
		return 0;

	{
		int s, e;
		size_t len = strlen(line);
		for (s = 0; s < len; s++) {
			if (!isspace(line[s]))
				break;
		}
		for (e = len-1; e >= 0; e--) {
			if (!isspace(line[e]))
				break;
		}
		strncpy(trimed, line+s, e-s+1);
	}

	if (trimed[0] == '#')
		return 0;

	sscanf(trimed, "%s%n", cmd, &nreads);

	return run_command(parser, cmd, trimed + nreads);
}

static int run_command(struct Parser *parser, const char *cmd, const char *argline)
{
	union Argument args[MAX_ARGS];
	struct TableEnt *ent;
	int err;
	ID id;

	if (strcmp(cmd, "OpenPlugin") == 0) {
		err = parse_args("s", argline, args, MAX_ARGS);
		if (err)
			return -1;

		printf(PROMPT"%s: [%s]\n", cmd, args[0].str);
		err = SiOpenPlugin(args[0].str);
		if (err == SI_FAIL) {
			set_errno(ERR_PSR_PLUGINNOTFOUND);
			return -1;
		}
	}
	else if (strcmp(cmd, "RenderScene") == 0) {
		err = parse_args("s", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s]\n", cmd, args[0].str);
		err = SiRenderScene(EntGetID(ent));
		if (err == SI_FAIL) {
			set_errno(ERR_PSR_FAILRENDER);
			return -1;
		}
	}
	else if (strcmp(cmd, "SaveFrameBuffer") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		err = SiSaveFrameBuffer(EntGetID(ent), args[1].str);
		if (err == SI_FAIL) {
			set_errno(ERR_PSR_FAILRENDER);
			return -1;
		}
	}
	else if (strcmp(cmd, "NewObjectInstance") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(ERR_PSR_NAMEEXISTS);
			return -1;
		}

		ent = TblLookup(parser->table, args[1].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewObjectInstance(EntGetID(ent));
		if (id == SI_BADID) {
			set_errno(ERR_PSR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewFrameBuffer") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(ERR_PSR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewFrameBuffer(args[1].str);
		if (id == SI_BADID) {
			set_errno(ERR_PSR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewRenderer") == 0) {
		err = parse_args("s", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(ERR_PSR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s]\n", cmd, args[0].str);
		id = SiNewRenderer();
		if (id == SI_BADID) {
			set_errno(ERR_PSR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewTexture") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(ERR_PSR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewTexture(args[1].str);
		if (id == SI_BADID) {
			set_errno(ERR_PSR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewCamera") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(ERR_PSR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: %s: %s\n", cmd, args[0].str, args[1].str);
		id = SiNewCamera(args[1].str);
		if (id == SI_BADID) {
			set_errno(ERR_PSR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewShader") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(ERR_PSR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewShader(args[1].str);
		if (id == SI_BADID) {
			set_errno(ERR_PSR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewCurve") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(ERR_PSR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewCurve(args[1].str);
		if (id == SI_BADID) {
			set_errno(ERR_PSR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewLight") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(ERR_PSR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewLight(args[1].str);
		if (id == SI_BADID) {
			set_errno(ERR_PSR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewMesh") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(ERR_PSR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewMesh(args[1].str);
		if (id == SI_BADID) {
			set_errno(ERR_PSR_FAILNEW);
			/* TODO better error message */
#if 0
			set_error_detail(SiGetErrorMessage(SiGetErrorNo()));
			append_error_detail(": ");
			append_error_detail(args[1].str);
#endif
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "AssignShader") == 0) {
		ID obj_id, shader_id;
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}
		obj_id = EntGetID(ent);

		ent = TblLookup(parser->table, args[1].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}
		shader_id = EntGetID(ent);

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		SiAssignShader(obj_id, shader_id);
	}
	else if (strcmp(cmd, "AssignTexture") == 0) {
		ID shader_id, tex_id;
		err = parse_args("sss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}
		shader_id = EntGetID(ent);

		ent = TblLookup(parser->table, args[2].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}
		tex_id = EntGetID(ent);

		printf(PROMPT"%s: [%s] [%s] [%s]\n", cmd, args[0].str, args[1].str, args[2].str);
		SiAssignTexture(shader_id, args[1].str, tex_id);
	}
	else if (strcmp(cmd, "AssignCamera") == 0) {
		ID renderer_id, camera_id;
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}
		renderer_id = EntGetID(ent);

		ent = TblLookup(parser->table, args[1].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}
		camera_id = EntGetID(ent);

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		SiAssignCamera(renderer_id, camera_id);
	}
	else if (strcmp(cmd, "AssignFrameBuffer") == 0) {
		ID renderer_id, framebuffer_id;
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}
		renderer_id = EntGetID(ent);

		ent = TblLookup(parser->table, args[1].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}
		framebuffer_id = EntGetID(ent);

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		SiAssignFrameBuffer(renderer_id, framebuffer_id);
	}
	else if (strcmp(cmd, "SetProperty1") == 0) {
		err = parse_args("ssf", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s] [%g]\n", cmd, args[0].str, args[1].str, args[2].dbl);
		SiSetProperty1(EntGetID(ent), args[1].str, args[2].dbl);
	}
	else if (strcmp(cmd, "SetProperty2") == 0) {
		err = parse_args("ssff", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s] [%g] [%g]\n", cmd, args[0].str, args[1].str,
				args[2].dbl, args[3].dbl);
		SiSetProperty2(EntGetID(ent), args[1].str, args[2].dbl, args[3].dbl);
	}
	else if (strcmp(cmd, "SetProperty3") == 0) {
		err = parse_args("ssfff", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s] [%g] [%g] [%g]\n", cmd, args[0].str, args[1].str,
				args[2].dbl, args[3].dbl, args[4].dbl);
		SiSetProperty3(EntGetID(ent), args[1].str, args[2].dbl, args[3].dbl, args[4].dbl);
	}
	else if (strcmp(cmd, "SetProperty4") == 0) {
		err = parse_args("ssffff", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(ERR_PSR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s] [%g] [%g] [%g] [%g]\n", cmd, args[0].str, args[1].str,
				args[2].dbl, args[3].dbl, args[4].dbl, args[5].dbl);
		SiSetProperty4(EntGetID(ent),
				args[1].str, args[2].dbl, args[3].dbl, args[4].dbl, args[5].dbl);
	}
	else {
		set_errno(ERR_PSR_UNKNOWNCMD);
		return -1;
	}

	return 0;
}

int PsrGetLineNo(const struct Parser *parser)
{
	return parser->line_no;
}

static int parse_args(const char *fmt, const char *arg, union Argument *args, int max_args)
{
	int i;
	int nargs, nreads, nscans;
	const char *nextarg;

	nextarg = arg;
	nargs = strlen(fmt);
	assert(nargs <= max_args);

	for (i = 0; i < nargs; i++) {
		nscans = 0;
		nreads = 0;

		switch (fmt[i]) {
		case 's':
			nscans = sscanf(nextarg, "%s%n", args[i].str, &nreads);
			break;
		case 'f':
			nscans = sscanf(nextarg, "%lf%n", &args[i].dbl, &nreads);
			break;
		default:
			assert(!"invalid format");
			break;
		}
		if (nscans < 1) {
			set_errno(ERR_PSR_FEWARGS);
			return -1;
		}

		nextarg += nreads;
	}
	nscans = 0;
	nreads = 0;
	nscans = sscanf(nextarg, "%*s%n", &nreads);
	if (nreads > 0) {
		set_errno(ERR_PSR_MANYARGS);
		return -1;
	}

	set_errno(ERR_PSR_NOERR);
	return 0;
}

#if 0
static void set_error_detail(const char *detail)
{
	size_t len;

	len = strlen(detail);
	strncpy(error_detail, detail, 1000);
}

static void append_error_detail(const char *detail)
{
	size_t len;

	len = strlen(detail);
	strncat(error_detail, detail, len);
}
#endif

