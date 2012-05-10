/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Plugin.h"
#include "OS.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Plugin {
	void *dso;
	struct PluginInfo info;
};

static int error_no = ERR_PLG_NOERR;
static void clear_pluginfo(struct PluginInfo *info);
static int is_valid_pluginfo(const struct PluginInfo *info);

int PlgGetErrorNo(void)
{
	return error_no;
}

const char *PlgGetErrorMessage(int err_no)
{
	static const char *errmsg[] = {
		"",                         /* ERR_PLG_NOERR */
		"No such plugin",           /* ERR_PLG_NOPLG */
		"No initialize function",   /* ERR_PLG_NOINIT */
		"Initialize plugin failed", /* ERR_PLG_INITFAIL */
		"Invalid plugin info",      /* ERR_PLG_BADINFO */
		"Cannot allocate memory",   /* ERR_PLG_NOMEM */
		"Fail to close dso"         /* ERR_PLG_FAILCLOSE */
	};
	static const int nerrs = (int) sizeof(errmsg)/sizeof(errmsg[0]);

	if (err_no >= nerrs) {
		fprintf(stderr, "Logic error: err_no %d is out of range\n", err_no);
		abort();
	}
	return errmsg[err_no];
}

struct Plugin *PlgOpen(const char *filename)
{
	void *tmpdso;
	struct Plugin *plg;
	struct PluginInfo info;
	PlgInitializeFn initialize_plugin;

	error_no = ERR_PLG_NOERR;
	tmpdso = OsDlopen(filename);
	if (tmpdso == NULL) {
		error_no = ERR_PLG_NOPLG;
		return NULL;
	}

	/* ISO C forbids conversion of object pointer to function pointer type.
	This does void pointer -> long int -> function pointer */
	initialize_plugin = (PlgInitializeFn) (long) OsDlsym(tmpdso, "Initialize");
	if (initialize_plugin == NULL) {
		error_no = ERR_PLG_NOINIT;
		OsDlclose(tmpdso);
		return NULL;
	}

	clear_pluginfo(&info);
	if (initialize_plugin(&info)) {
		error_no = ERR_PLG_INITFAIL;
		OsDlclose(tmpdso);
		return NULL;
	}

	if (!is_valid_pluginfo(&info)) {
		error_no = ERR_PLG_BADINFO;
		OsDlclose(tmpdso);
		return NULL;
	}

	plg = (struct Plugin *) malloc(sizeof(struct Plugin));
	if (plg == NULL) {
		error_no = ERR_PLG_NOMEM;
		OsDlclose(tmpdso);
		return NULL;
	}

	/* commit */
	plg->dso = tmpdso;
	plg->info = info;

	return plg;
}

int PlgClose(struct Plugin *plg)
{
	int err;

	err = OsDlclose(plg->dso);
	free(plg);

	if (err) {
		error_no = ERR_PLG_FAILCLOSE;
		return -1;
	} else {
		error_no = ERR_PLG_NOERR;
		return 0;
	}
}

void *PlgCreateInstance(const struct Plugin *plg)
{
	return plg->info.create_instance();
}

void PlgDeleteInstance(const struct Plugin *plg, void *obj)
{
	plg->info.delete_instance(obj);
}

const struct MetaInfo *PlgMetainfo(const struct Plugin *plg)
{
	return plg->info.meta;
}

const void *PlgGetVtable(const struct Plugin *plg)
{
	return plg->info.vtbl;
}

const char *PlgGetName(const struct Plugin *plg)
{
	return plg->info.name;
}

void clear_pluginfo(struct PluginInfo *info)
{
	info->api_version = 0;
	info->name = NULL;
	info->create_instance = NULL;
	info->delete_instance = NULL;
	info->vtbl = NULL;
	info->meta = NULL;
}

int is_valid_pluginfo(const struct PluginInfo *info)
{
	if (info->api_version != PLUGIN_API_VERSION ||
		info->name == NULL ||
		info->create_instance == NULL ||
		info->delete_instance == NULL ||
		info->vtbl == NULL ||
		info->meta == NULL)
		return 0;
	else
		return 1;
}

