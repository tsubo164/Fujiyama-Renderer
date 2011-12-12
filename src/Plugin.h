/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#define PLUGIN_API_VERSION 1

struct Plugin;
struct PluginInfo;
struct MetaInfo;

typedef int (*PlgInitializeFn)(struct PluginInfo *info);
typedef void *(*PlgCreateInstanceFn)(void);
typedef void (*PlgDeleteInstanceFn)(void *obj);

enum PlgErrorNo {
	ERR_PLG_NOERR = 0,
	ERR_PLG_NOPLG,
	ERR_PLG_NOINIT,
	ERR_PLG_INITFAIL,
	ERR_PLG_BADINFO,
	ERR_PLG_NOMEM,
	ERR_PLG_FAILCLOSE
};

struct PluginInfo {
	int api_version;
	const char *name;
	PlgCreateInstanceFn create_instance;
	PlgDeleteInstanceFn delete_instance;
	const void *vtbl;
	const struct MetaInfo *meta;
};

struct MetaInfo {
	const char *name;
	const char *data;
};

extern int PlgGetErrorNo(void);
extern const char *PlgGetErrorMessage(int err_no);

extern struct Plugin *PlgOpen(const char *filename);
extern int PlgClose(struct Plugin *plg);

extern void *PlgCreateInstance(const struct Plugin *plg);
extern void PlgDeleteInstance(const struct Plugin *plg, void *obj);

extern const struct MetaInfo *PlgMetainfo(const struct Plugin *plg);
extern const void *PlgGetVtable(const struct Plugin *plg);
extern const char *PlgGetName(const struct Plugin *plg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

