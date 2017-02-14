// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_procedure.h"
#include "fj_timer.h"

#include <cstdio>
#include <cassert>

namespace fj {

static int error_no = PRC_ERR_NOERR;
static void set_error(int err);

Procedure::Procedure() :
    self_(NULL),
    vptr_(NULL),
    plugin_(NULL)
{
}

Procedure::~Procedure()
{
  // TODO need NullPlugin?
  if (plugin_ != NULL) {
    plugin_->DeleteInstance(self_);
  }
}

int Procedure::Initialize(Plugin *plugin)
{
  if (!plugin->TypeMatch(PROCEDURE_PLUGIN_TYPE)) {
    set_error(PRC_ERR_TYPE_NOT_MATCH);
    return -1;
  }

  void *tmpobj = plugin->CreateInstance();
  if (tmpobj == NULL) {
    set_error(PRC_ERR_NOOBJ);
    return -1;
  }

  const void *tmpvtbl = plugin->GetVtable();
  if (tmpvtbl == NULL) {
    set_error(PRC_ERR_NOVTBL);
    plugin->DeleteInstance(tmpobj);
    return -1;
  }

  // commit
  self_ = tmpobj;
  vptr_ = reinterpret_cast<const ProcedureFunctionTable *>(tmpvtbl);
  plugin_ = plugin;
  set_error(PRC_ERR_NOERR);

  return 0;
}

int Procedure::Run()
{
  /* TODO come up with the best place to put message */
  printf("Running Procedure ...\n");

  Timer timer;
  timer.Start();

  const int err = vptr_->MyRun(self_);

  Elapse elapse = timer.GetElapse();

  if (err) {
    printf("Error: %dh %dm %ds\n", elapse.hour, elapse.min, elapse.sec);
    return -1;
  } else {
    printf("Done: %dh %dm %ds\n", elapse.hour, elapse.min, elapse.sec);
    return 0;
  }
}

const Property *Procedure::GetPropertyList() const
{
  // TODO need NullPlugin?
  if (plugin_ == NULL)
    return NULL;

  return plugin_->GetPropertyList();
}

int Procedure::SetProperty(const std::string &prop_name, const PropertyValue &src_data)
{
  const Property *prc_props = GetPropertyList();
  const Property *dst_prop  = PropFind(prc_props, src_data.type, prop_name.c_str());

  if (dst_prop == NULL) {
    return -1;
  }

  assert(dst_prop->SetProperty != NULL);
  return dst_prop->SetProperty(self_, &src_data);
}

static void set_error(int err)
{
  error_no = err;
}

} // namespace xxx
