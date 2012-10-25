/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Parser.h"
#include "SceneInterface.h"
#include "Numeric.h"
#include "String.h"
#include "Table.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#define PROMPT "-- "

enum PsrErroNo {
	PSR_ERR_NOERR = 1024, /* offset to avoid conflict with SI_ERR */
	PSR_ERR_UNKNOWNCMD,
	PSR_ERR_MANYARGS,
	PSR_ERR_FEWARGS,
	PSR_ERR_NAMEEXISTS,
	PSR_ERR_NAMENOTFOUND,
	PSR_ERR_FAILSETPROP,
	PSR_ERR_PLUGINNOTFOUND,
	PSR_ERR_FAILNEW,
	PSR_ERR_FAILRENDER
};

enum { MAX_ARGS = 6 };

union Argument {
	char str[1024];
	double dbl;
};

struct Parser {
	int line_no;
	struct Table *table;
};

static const char *error_message = NULL;
static int parser_errno = PSR_ERR_NOERR;

static int run_command(struct Parser *parser, const char *cmd, const char *arg);
static int parse_args(const char *fmt, const char *arg, union Argument *args, int max_args);
static void set_errno(int err);
static void set_error_message(int current_errno);

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

const char *PsrGetErrorMessage(void)
{
	const int err_no = parser_errno;
	/* TODO this is a temporal solution for plugin errors */
	/* TODO other errors will be handled in the same ways */
	if (err_no == PSR_ERR_PLUGINNOTFOUND) {
		if (error_message == NULL)
			return "";

		return error_message;
	} else {
		set_error_message(err_no);
		return error_message;
	}
}

int PsrParseLine(struct Parser *parser, const char *line)
{
	char trimed[1024] = {'\0'};
	char cmd[64] = {'\0'};
	int nreads;

	parser->line_no++;

	if (line[0] == '\n')
		return 0;

	{
		size_t s, e;
		size_t ncpy;
		size_t len = strlen(line);
		for (s = 0; s < len; s++) {
			if (!isspace(line[s]))
				break;
		}
		for (e = len-1; e >= 0; e--) {
			if (!isspace(line[e]))
				break;
		}
		ncpy = e-s+1;
		ncpy = MIN(ncpy, 1000);
		StrCopyAndTerminate(trimed, line+s, ncpy);
	}

	if (trimed[0] == '#')
		return 0;

	sscanf(trimed, "%s%n", cmd, &nreads);

	return run_command(parser, cmd, trimed + nreads);
}

static int run_command(struct Parser *parser, const char *cmd, const char *argline)
{
	union Argument args[MAX_ARGS];
	struct TableEnt *ent = NULL;
	int err = 0;
	ID id = SI_BADID;

	if (strcmp(cmd, "OpenPlugin") == 0) {
		err = parse_args("s", argline, args, MAX_ARGS);
		if (err)
			return -1;

		printf(PROMPT"%s: [%s]\n", cmd, args[0].str);
		err = SiOpenPlugin(args[0].str);
		if (err == SI_FAIL) {
			set_error_message(SiGetErrorNo());
			set_errno(PSR_ERR_PLUGINNOTFOUND);
			return -1;
		}
	}
	else if (strcmp(cmd, "RenderScene") == 0) {
		err = parse_args("s", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s]\n", cmd, args[0].str);
		err = SiRenderScene(EntGetID(ent));
		if (err == SI_FAIL) {
			set_errno(PSR_ERR_FAILRENDER);
			return -1;
		}
	}
	else if (strcmp(cmd, "SaveFrameBuffer") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		err = SiSaveFrameBuffer(EntGetID(ent), args[1].str);
		if (err == SI_FAIL) {
			set_errno(PSR_ERR_FAILRENDER);
			return -1;
		}
	}
	else if (strcmp(cmd, "RunProcedure") == 0) {
		err = parse_args("s", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s]\n", cmd, args[0].str);
		err = SiRunProcedure(EntGetID(ent));
		if (err == SI_FAIL) {
			set_errno(PSR_ERR_FAILRENDER);
			return -1;
		}
	}
	else if (strcmp(cmd, "NewTurbulence") == 0) {
		err = parse_args("s", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s]\n", cmd, args[0].str);
		id = SiNewTurbulence();
		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewObjectInstance") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		ent = TblLookup(parser->table, args[1].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewObjectInstance(EntGetID(ent));
		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewFrameBuffer") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewFrameBuffer(args[1].str);
		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewProcedure") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewProcedure(args[1].str);
		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewRenderer") == 0) {
		err = parse_args("s", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s]\n", cmd, args[0].str);
		id = SiNewRenderer();
		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewTexture") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewTexture(args[1].str);
		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewCamera") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: %s: %s\n", cmd, args[0].str, args[1].str);
		id = SiNewCamera(args[1].str);
		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewShader") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewShader(args[1].str);
		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewVolume") == 0) {
		err = parse_args("s", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s]\n", cmd, args[0].str);
		id = SiNewVolume();
		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewCurve") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewCurve(args[1].str);
		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewLight") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewLight(args[1].str);
		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
			return -1;
		}

		TblAdd(parser->table, args[0].str, id);
	}
	else if (strcmp(cmd, "NewMesh") == 0) {
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		if (TblLookup(parser->table, args[0].str)) {
			set_errno(PSR_ERR_NAMEEXISTS);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		id = SiNewMesh(args[1].str);

		if (id == SI_BADID) {
			set_errno(PSR_ERR_FAILNEW);
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
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		obj_id = EntGetID(ent);

		ent = TblLookup(parser->table, args[1].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		shader_id = EntGetID(ent);

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		err = SiAssignShader(obj_id, shader_id);
		if (err) {
			set_errno(PSR_ERR_FAILSETPROP);
			return -1;
		}
	}
	else if (strcmp(cmd, "AssignTexture") == 0) {
		ID shader_id, tex_id;
		err = parse_args("sss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		shader_id = EntGetID(ent);

		ent = TblLookup(parser->table, args[2].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		tex_id = EntGetID(ent);

		printf(PROMPT"%s: [%s] [%s] [%s]\n", cmd, args[0].str, args[1].str, args[2].str);
		err = SiAssignTexture(shader_id, args[1].str, tex_id);
		if (err) {
			set_errno(PSR_ERR_FAILSETPROP);
			return -1;
		}
	}
	else if (strcmp(cmd, "AssignCamera") == 0) {
		ID renderer_id, camera_id;
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		renderer_id = EntGetID(ent);

		ent = TblLookup(parser->table, args[1].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		camera_id = EntGetID(ent);

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		err = SiAssignCamera(renderer_id, camera_id);
		if (err) {
			set_errno(PSR_ERR_FAILSETPROP);
			return -1;
		}
	}
	else if (strcmp(cmd, "AssignFrameBuffer") == 0) {
		ID renderer_id, framebuffer_id;
		err = parse_args("ss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		renderer_id = EntGetID(ent);

		ent = TblLookup(parser->table, args[1].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		framebuffer_id = EntGetID(ent);

		printf(PROMPT"%s: [%s] [%s]\n", cmd, args[0].str, args[1].str);
		err = SiAssignFrameBuffer(renderer_id, framebuffer_id);
		if (err) {
			set_errno(PSR_ERR_FAILSETPROP);
			return -1;
		}
	}
	else if (strcmp(cmd, "AssignTurbulence") == 0) {
		ID id, turbulence_id;
		err = parse_args("sss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		id = EntGetID(ent);

		ent = TblLookup(parser->table, args[2].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		turbulence_id = EntGetID(ent);

		printf(PROMPT"%s: [%s] [%s] [%s]\n", cmd, args[0].str, args[1].str, args[2].str);
		err = SiAssignTurbulence(id, args[1].str, turbulence_id);
		if (err) {
			set_errno(PSR_ERR_FAILSETPROP);
			return -1;
		}
	}
	else if (strcmp(cmd, "AssignVolume") == 0) {
		ID id, volume_id;
		err = parse_args("sss", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		id = EntGetID(ent);

		ent = TblLookup(parser->table, args[2].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}
		volume_id = EntGetID(ent);

		printf(PROMPT"%s: [%s] [%s] [%s]\n", cmd, args[0].str, args[1].str, args[2].str);
		err = SiAssignVolume(id, args[1].str, volume_id);
		if (err) {
			set_errno(PSR_ERR_FAILSETPROP);
			return -1;
		}
	}
	else if (strcmp(cmd, "SetProperty1") == 0) {
		err = parse_args("ssf", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s] [%g]\n", cmd, args[0].str, args[1].str, args[2].dbl);
		err = SiSetProperty1(EntGetID(ent), args[1].str, args[2].dbl);
		if (err) {
			set_errno(PSR_ERR_FAILSETPROP);
			return -1;
		}
	}
	else if (strcmp(cmd, "SetProperty2") == 0) {
		err = parse_args("ssff", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s] [%g] [%g]\n", cmd, args[0].str, args[1].str,
				args[2].dbl, args[3].dbl);
		err = SiSetProperty2(EntGetID(ent), args[1].str, args[2].dbl, args[3].dbl);
		if (err) {
			set_errno(PSR_ERR_FAILSETPROP);
			return -1;
		}
	}
	else if (strcmp(cmd, "SetProperty3") == 0) {
		err = parse_args("ssfff", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s] [%g] [%g] [%g]\n", cmd, args[0].str, args[1].str,
				args[2].dbl, args[3].dbl, args[4].dbl);
		/* TODO check set property error */
		err = SiSetProperty3(EntGetID(ent), args[1].str, args[2].dbl, args[3].dbl, args[4].dbl);
		if (err) {
			set_errno(PSR_ERR_FAILSETPROP);
			return -1;
		}
	}
	else if (strcmp(cmd, "SetProperty4") == 0) {
		err = parse_args("ssffff", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s] [%g] [%g] [%g] [%g]\n", cmd, args[0].str, args[1].str,
				args[2].dbl, args[3].dbl, args[4].dbl, args[5].dbl);
		err = SiSetProperty4(EntGetID(ent),
				args[1].str, args[2].dbl, args[3].dbl, args[4].dbl, args[5].dbl);
		if (err) {
			set_errno(PSR_ERR_FAILSETPROP);
			return -1;
		}
	}
	else if (strcmp(cmd, "ShowPropertyList") == 0) {
		err = parse_args("s", argline, args, MAX_ARGS);
		if (err)
			return -1;

		printf(PROMPT"%s: [%s]\n", cmd, args[0].str);
		{
			const char **prop_types = NULL;
			const char **prop_names = NULL;
			const char *type_name = args[0].str;
			int nprops = 0;
			int err = 0;
			int i = 0;

			err = SiGetPropertyList(type_name, &prop_types, &prop_names, &nprops);
			if (err) {
				printf("#   No property is available for %s\n", type_name);
				printf("#   Make sure the type name is correct.\n");
				printf("#   If you requested properties for a plugin,\n");
				printf("#   make sure it is opened before calling this command.\n");
				return -1;
			}

			printf("#   %s Properties\n", type_name);
			for (i = 0; i < nprops; i++) {
				printf("#   %15.15s : %-20.20s\n", prop_types[i], prop_names[i]);
			}
		}
	}
	/* TODO TEST SetSampleProperty3 */
	else if (strcmp(cmd, "SetSampleProperty3") == 0) {
		err = parse_args("ssffff", argline, args, MAX_ARGS);
		if (err)
			return -1;

		ent = TblLookup(parser->table, args[0].str);
		if (ent == NULL) {
			set_errno(PSR_ERR_NAMENOTFOUND);
			return -1;
		}

		printf(PROMPT"%s: [%s] [%s] [%g] [%g] [%g] [%g]\n", cmd, args[0].str, args[1].str,
				args[2].dbl, args[3].dbl, args[4].dbl, args[5].dbl);
		/* TODO check set property error */
		err = SiSetSampleProperty3(EntGetID(ent),
				args[1].str, args[2].dbl, args[3].dbl, args[4].dbl, args[5].dbl);
		if (err) {
			set_errno(PSR_ERR_FAILSETPROP);
			return -1;
		}
	}
	else {
		set_errno(PSR_ERR_UNKNOWNCMD);
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
			set_errno(PSR_ERR_FEWARGS);
			return -1;
		}

		nextarg += nreads;
	}
	nscans = 0;
	nreads = 0;
	nscans = sscanf(nextarg, "%*s%n", &nreads);
	if (nreads > 0) {
		set_errno(PSR_ERR_MANYARGS);
		return -1;
	}

	set_errno(PSR_ERR_NOERR);
	return 0;
}

static void set_errno(int err)
{
	parser_errno = err;
}

/* TODO should move this to SceneInterface.c? */
struct SiError {
	int number;
	const char *message;
};

static void set_error_message(int current_errno)
{
	static const struct SiError errors[] = {
		/* from Parser */
		{PSR_ERR_NOERR, ""},
		{PSR_ERR_UNKNOWNCMD,     "unknown command"},
		{PSR_ERR_MANYARGS,       "too many arguments"},
		{PSR_ERR_FEWARGS,        "too few arguments"},
		{PSR_ERR_NAMEEXISTS,     "name already exists"},
		{PSR_ERR_NAMENOTFOUND,   "name not found"},
		{PSR_ERR_FAILSETPROP,    "set property faided"},
		{PSR_ERR_PLUGINNOTFOUND, "plugin not found"},
		{PSR_ERR_FAILNEW,        "new entry failed"},
		{PSR_ERR_FAILRENDER ,    "render faided"},
		/* from SceneInterface */
		{SI_ERR_PLUGIN_NOT_FOUND,           "plugin not found"},
		{SI_ERR_INIT_PLUGIN_FUNC_NOT_EXIST, "initialize plugin function not exit"},
		{SI_ERR_INIT_PLUGIN_FUNC_FAIL,      "initialize plugin function failed"},
		{SI_ERR_BAD_PLUGIN_INFO,            "invalid plugin info in the plugin"},
		{SI_ERR_CLOSE_PLUGIN_FAIL,          "close plugin function failed"},
		{SI_ERR_NONE, ""}
	};
	const struct SiError *error = NULL;

	for (error = errors; error->number != SI_ERR_NONE; error++) {
		if (error->number == current_errno) {
			error_message = error->message;
			break;
		}
	}
}

