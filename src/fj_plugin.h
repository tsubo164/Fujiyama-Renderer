// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_PLUGIN_H
#define FJ_PLUGIN_H

#include "fj_compatibility.h"
#include <string>
#include <cstddef>

#define PLUGIN_API_VERSION 1

namespace fj {

class PluginInfo;
class Property;
class MetaInfo;

typedef int (*PlgInitializeFn)(PluginInfo *info);
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

class PluginInfo {
public:
  PluginInfo() :
      api_version(0),
      plugin_type(NULL),
      plugin_name(NULL),
      create_instance(NULL),
      delete_instance(NULL),
      vtbl(NULL),
      properties(NULL),
      meta(NULL)
  {}
  ~PluginInfo() {}
  
  int api_version;
  const char *plugin_type;
  const char *plugin_name;
  PlgCreateInstanceFn create_instance;
  PlgDeleteInstanceFn delete_instance;
  const void *vtbl;
  const Property *properties;
  const MetaInfo *meta;
};

class MetaInfo {
public:
#if 0
  MetaInfo() :
      name(NULL),
      data(NULL)
  {}
  ~MetaInfo() {}
#endif

  const char *name;
  const char *data;
};

class Plugin {
public:
  Plugin();
  ~Plugin();

  int Open(const std::string &filename);
  void Close();

  void *CreateInstance() const;
  void DeleteInstance(void *instance) const;

  const Property *GetPropertyList() const;
  const MetaInfo *Metainfo() const;
  const void *GetVtable() const;
  const char *GetName() const;
  const char *GetType() const;
  int TypeMatch(const char *type) const;

private:
  void *dso_;
  PluginInfo info_;
};

FJ_API Plugin *PlgOpen(const char *filename);
FJ_API int PlgClose(Plugin *plugin);

FJ_API int PlgSetupInfo(PluginInfo *info,
    int api_version,
    const char *plugin_type,
    const char *plugin_name,
    PlgCreateInstanceFn create_instance,
    PlgDeleteInstanceFn delete_instance,
    const void *vtbl,
    const Property *properties,
    const MetaInfo *meta);

FJ_API int PlgGetErrorNo(void);

} // namespace xxx

#endif // FJ_XXX_H
