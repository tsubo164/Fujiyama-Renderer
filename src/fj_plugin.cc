// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_plugin.h"
#include "fj_compatibility.h"
#include "fj_os.h"

#include <iostream>

#include <cstdio>
#include <cstring>

namespace fj {

static int error_no = PLG_ERR_NONE;
static int is_valid_pluginfo(const PluginInfo *info);
static void set_errno(int err_no);

Plugin::Plugin() : dso_(NULL), info_(), instance_list_()
{
}

Plugin::~Plugin()
{
  Close();
}

int Plugin::Open(const std::string &filename)
{
  set_errno(PLG_ERR_NONE);

  void *tmpdso = OsDlopen(filename.c_str());
  if (tmpdso == NULL) {
    set_errno(PLG_ERR_PLUGIN_NOT_FOUND);
    return -1;
  }

  // ISO C forbids conversion of object pointer to function pointer type.
  // This does void pointer -> uintptr_t -> function pointer
  PlgInitializeFn initialize_plugin =
      reinterpret_cast<PlgInitializeFn>(
          reinterpret_cast<uintptr_t>(OsDlsym(tmpdso, "Initialize")));
  if (initialize_plugin == NULL) {
    set_errno(PLG_ERR_INIT_PLUGIN_FUNC_NOT_EXIST);
    OsDlclose(tmpdso);
    return -1;
  }

  PluginInfo info;

  const int err = initialize_plugin(&info);
  if (err) {
    set_errno(PLG_ERR_INIT_PLUGIN_FUNC_FAIL);
    OsDlclose(tmpdso);
    return -1;
  }

  // TODO this is checked in initialize_plugin
  if (!is_valid_pluginfo(&info)) {
    set_errno(PLG_ERR_BAD_PLUGIN_INFO);
    OsDlclose(tmpdso);
    return -1;
  }

  // commit
  dso_ = tmpdso;
  info_ = info;
  return 0;
}

void Plugin::Close()
{
  for (std::size_t i = 0; i < instance_list_.size(); i++) {
    DeleteInstance(instance_list_[i]);
  }
  OsDlclose(dso_);
}

void *Plugin::CreateInstance()
{
  void *instance = info_.create_instance();
  instance_list_.push_back(instance);
  return instance;
}

void Plugin::DeleteInstance(void *instance) const
{
  info_.delete_instance(instance);
}

const Property *Plugin::GetPropertyList() const
{
  return info_.property_list;
}

const MetaInfo *Plugin::Metainfo() const
{
  return info_.meta;
}

const char *Plugin::GetName() const
{
  return info_.plugin_name;
}

const char *Plugin::GetType() const
{
  return info_.plugin_type;
}

int Plugin::TypeMatch(const char *type) const
{
  return strcmp(GetType(), type) == 0;
}

Plugin *PlgOpen(const char *filename)
{
  Plugin *plugin = new Plugin();
  plugin->Open(filename);
  return plugin;
}

int PlgClose(Plugin *plugin)
{
  delete plugin;
  return 0;
}

int PlgSetupInfo(PluginInfo *info,
    int api_version,
    const char *plugin_type,
    const char *plugin_name,
    PlgCreateInstanceFn create_instance,
    PlgDeleteInstanceFn delete_instance,
    const Property *property_list,
    const MetaInfo *meta)
{
  info->api_version = api_version;
  info->plugin_type = plugin_type;
  info->plugin_name = plugin_name;
  info->create_instance = create_instance;
  info->delete_instance = delete_instance;
  info->property_list = property_list;
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

static int is_valid_pluginfo(const PluginInfo *info)
{
  if (info->api_version != PLUGIN_API_VERSION ||
    info->plugin_type == NULL ||
    info->plugin_name == NULL ||
    info->create_instance == NULL ||
    info->delete_instance == NULL ||
    info->property_list == NULL ||
    info->meta == NULL)
    return 0;
  else
    return 1;
}

} // namespace xxx
