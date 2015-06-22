// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"
#include "fj_vector.h"
#include <cassert>

namespace fj {

static const Color NO_SHADER_COLOR(.5, 1., 0.);
static int error_no = SHD_ERR_NOERR;
static void set_error(int err);

Shader::Shader() :
    self_(NULL),
    vptr_(NULL),
    plugin_(NULL)
{
}

Shader::~Shader()
{
  // TODO need NullPlugin?
  if (plugin_ != NULL) {
    plugin_->DeleteInstance(self_);
  }
}

int Shader::Initialize(const Plugin *plugin)
{
  if (!plugin->TypeMatch(SHADER_PLUGIN_TYPE)) {
    set_error(SHD_ERR_TYPE_NOT_MATCH);
    return -1;
  }

  void *tmpobj = plugin->CreateInstance();
  if (tmpobj == NULL) {
    set_error(SHD_ERR_NOOBJ);
    return -1;
  }

  const void *tmpvtbl = plugin->GetVtable();
  if (tmpvtbl == NULL) {
    set_error(SHD_ERR_NOVTBL);
    plugin->DeleteInstance(tmpobj);
    return -1;
  }

  // commit
  self_ = tmpobj;
  vptr_ = reinterpret_cast<const ShaderFunctionTable *>(tmpvtbl);
  plugin_ = plugin;
  set_error(SHD_ERR_NOERR);

  return 0;
}

void Shader::Evaluate(const TraceContext &cxt, const SurfaceInput &in,
    SurfaceOutput *out) const
{
  if (vptr_ == NULL) {
    out->Cs = NO_SHADER_COLOR;
    out->Os = 1;
    return;
  }
  vptr_->MyEvaluate(self_, &cxt, &in, out);
}

const Property *Shader::GetPropertyList() const
{
  // TODO need NullPlugin?
  if (plugin_ == NULL)
    return NULL;

  return plugin_->GetPropertyList();
}

int Shader::SetProperty(const std::string &prop_name, const PropertyValue &src_data) const
{
  const Property *shd_props = GetPropertyList();
  const Property *dst_prop = PropFind(shd_props, src_data.type, prop_name.c_str());

  if (dst_prop == NULL)
    return -1;

  assert(dst_prop->SetProperty != NULL);
  return dst_prop->SetProperty(self_, &src_data);
}

static void set_error(int err)
{
  error_no = err;
}

} // namespace xxx
