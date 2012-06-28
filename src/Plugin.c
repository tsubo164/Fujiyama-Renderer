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
	struct Plugin *plugin;
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

	/* TODO this is checked in initialize_plugin */
	if (!is_valid_pluginfo(&info)) {
		error_no = ERR_PLG_BADINFO;
		OsDlclose(tmpdso);
		return NULL;
	}

	plugin = (struct Plugin *) malloc(sizeof(struct Plugin));
	if (plugin == NULL) {
		error_no = ERR_PLG_NOMEM;
		OsDlclose(tmpdso);
		return NULL;
	}

	/* commit */
	plugin->dso = tmpdso;
	plugin->info = info;

	return plugin;
}

int PlgClose(struct Plugin *plugin)
{
	int err;

	err = OsDlclose(plugin->dso);
	free(plugin);

	if (err) {
		error_no = ERR_PLG_FAILCLOSE;
		return -1;
	} else {
		error_no = ERR_PLG_NOERR;
		return 0;
	}
}

void *PlgCreateInstance(const struct Plugin *plugin)
{
	return plugin->info.create_instance();
}

void PlgDeleteInstance(const struct Plugin *plugin, void *obj)
{
	plugin->info.delete_instance(obj);
}

const struct Property *PlgGetPropertyList(const struct Plugin *plugin)
{
	return plugin->info.properties;
}

const struct MetaInfo *PlgMetainfo(const struct Plugin *plugin)
{
	return plugin->info.meta;
}

const void *PlgGetVtable(const struct Plugin *plugin)
{
	return plugin->info.vtbl;
}

const char *PlgGetName(const struct Plugin *plugin)
{
	return plugin->info.plugin_name;
}

const char *PlgGetType(const struct Plugin *plugin)
{
	return plugin->info.plugin_type;
}

int PlgTypeMatch(const struct Plugin *plugin, const char *type)
{
	return strcmp(PlgGetType(plugin), type) == 0;
}

int PlgSetupInfo(struct PluginInfo *info,
		int api_version,
		const char *plugin_type,
		const char *plugin_name,
		PlgCreateInstanceFn create_instance,
		PlgDeleteInstanceFn delete_instance,
		const void *vtbl,
		const struct Property *properties,
		const struct MetaInfo *meta)
{
	info->api_version = api_version;
	info->plugin_type = plugin_type;
	info->plugin_name = plugin_name;
	info->create_instance = create_instance;
	info->delete_instance = delete_instance;
	info->vtbl = vtbl;
	info->properties = properties;
	info->meta = meta;

	if (is_valid_pluginfo(info))
		return 0;
	else
		return -1;
}

void clear_pluginfo(struct PluginInfo *info)
{
	info->api_version = 0;
	info->plugin_type = NULL;
	info->plugin_name = NULL;
	info->create_instance = NULL;
	info->delete_instance = NULL;
	info->vtbl = NULL;
	info->properties = NULL;
	info->meta = NULL;
}

int is_valid_pluginfo(const struct PluginInfo *info)
{
	if (info->api_version != PLUGIN_API_VERSION ||
		info->plugin_type == NULL ||
		info->plugin_name == NULL ||
		info->create_instance == NULL ||
		info->delete_instance == NULL ||
		info->vtbl == NULL ||
		info->properties == NULL ||
		info->meta == NULL)
		return 0;
	else
		return 1;
}

