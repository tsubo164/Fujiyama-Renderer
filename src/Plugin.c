/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Plugin.h"
#include "OS.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int error_no = PLG_ERR_NONE;
static void clear_pluginfo(struct PluginInfo *info);
static int is_valid_pluginfo(const struct PluginInfo *info);
static void set_errno(int err_no);

struct Plugin {
	void *dso;
	struct PluginInfo info;
};

struct Plugin *PlgOpen(const char *filename)
{
	int err = 0;
	void *tmpdso = NULL;
	struct Plugin *plugin = NULL;
	struct PluginInfo info;
	PlgInitializeFn initialize_plugin = NULL;

	set_errno(PLG_ERR_NONE);
	tmpdso = OsDlopen(filename);
	if (tmpdso == NULL) {
		set_errno(PLG_ERR_NOPLUGIN);
		goto plugin_error;
	}

	/* ISO C forbids conversion of object pointer to function pointer type.
	This does void pointer -> long int -> function pointer */
	initialize_plugin = (PlgInitializeFn) (long) OsDlsym(tmpdso, "Initialize");
	if (initialize_plugin == NULL) {
		set_errno(PLG_ERR_NOINITFUNC);
		goto plugin_error;
	}

	clear_pluginfo(&info);
	err = initialize_plugin(&info);
	if (err) {
		set_errno(PLG_ERR_INITFAIL);
		goto plugin_error;
	}

	/* TODO this is checked in initialize_plugin */
	if (!is_valid_pluginfo(&info)) {
		set_errno(PLG_ERR_BADINFO);
		goto plugin_error;
	}

	plugin = (struct Plugin *) malloc(sizeof(struct Plugin));
	if (plugin == NULL) {
		set_errno(PLG_ERR_NOMEM);
		goto plugin_error;
	}

	/* commit */
	plugin->dso = tmpdso;
	plugin->info = info;
	return plugin;

plugin_error:
	OsDlclose(tmpdso);
	return NULL;
}

int PlgClose(struct Plugin *plugin)
{
	const int err = OsDlclose(plugin->dso);

	free(plugin);

	if (err) {
		set_errno(PLG_ERR_CLOSEFAIL);
		return -1;
	} else {
		set_errno(PLG_ERR_NONE);
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

int PlgGetErrorNo(void)
{
	return error_no;
}

static void set_errno(int err_no)
{
	error_no = err_no;
}

static void clear_pluginfo(struct PluginInfo *info)
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

static int is_valid_pluginfo(const struct PluginInfo *info)
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

