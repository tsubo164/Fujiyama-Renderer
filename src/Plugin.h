/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef PLUGIN_H
#define PLUGIN_H

#define PLUGIN_API_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

struct Plugin;
struct PluginInfo;
struct Property;
struct MetaInfo;

typedef int (*PlgInitializeFn)(struct PluginInfo *info);
typedef void *(*PlgCreateInstanceFn)(void);
typedef void (*PlgDeleteInstanceFn)(void *obj);

enum PlgErrorNo {
	PLG_ERR_NONE = 0,
	PLG_ERR_NOPLUGIN,   /* plugin not found */
	PLG_ERR_NOINITFUNC, /* init func not exist */
	PLG_ERR_INITFAIL,   /* init func failed */
	PLG_ERR_BADINFO,    /* bad plugin info */
	PLG_ERR_NOMEM,      /* no memory for plugin */
	PLG_ERR_CLOSEFAIL   /* close plugin failed */
};

struct PluginInfo {
	int api_version;
	const char *plugin_type;
	const char *plugin_name;
	PlgCreateInstanceFn create_instance;
	PlgDeleteInstanceFn delete_instance;
	const void *vtbl;
	const struct Property *properties;
	const struct MetaInfo *meta;
};

struct MetaInfo {
	const char *name;
	const char *data;
};

extern struct Plugin *PlgOpen(const char *filename);
extern int PlgClose(struct Plugin *plugin);

extern void *PlgCreateInstance(const struct Plugin *plugin);
extern void PlgDeleteInstance(const struct Plugin *plugin, void *obj);

extern const struct Property *PlgGetPropertyList(const struct Plugin *plugin);
extern const struct MetaInfo *PlgMetainfo(const struct Plugin *plugin);
extern const void *PlgGetVtable(const struct Plugin *plugin);
extern const char *PlgGetName(const struct Plugin *plugin);
extern const char *PlgGetType(const struct Plugin *plugin);
extern int PlgTypeMatch(const struct Plugin *plugin, const char *type);

extern int PlgSetupInfo(struct PluginInfo *info,
		int api_version,
		const char *plugin_type,
		const char *plugin_name,
		PlgCreateInstanceFn create_instance,
		PlgDeleteInstanceFn delete_instance,
		const void *vtbl,
		const struct Property *properties,
		const struct MetaInfo *meta);

extern int PlgGetErrorNo(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

