/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_PLUGIN_H
#define FJ_PLUGIN_H

#define PLUGIN_API_VERSION 1

namespace fj {

struct Plugin;
struct PluginInfo;
struct Property;
struct MetaInfo;

typedef int (*PlgInitializeFn)(struct PluginInfo *info);
typedef void *(*PlgCreateInstanceFn)(void);
typedef void (*PlgDeleteInstanceFn)(void *obj);

enum PlgErrorNo {
  PLG_ERR_NONE = 0,
  PLG_ERR_PLUGIN_NOT_FOUND,
  PLG_ERR_INIT_PLUGIN_FUNC_NOT_EXIST,
  PLG_ERR_INIT_PLUGIN_FUNC_FAIL,
  PLG_ERR_BAD_PLUGIN_INFO,
  PLG_ERR_CLOSE_PLUGIN_FAIL,
  PLG_ERR_NO_MEMORY
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

} // namespace xxx

#endif /* FJ_XXX_H */
