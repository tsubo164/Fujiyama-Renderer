// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"
#include "fj_vector.h"
#include <cassert>

namespace fj {

static const Color NO_SHADER_COLOR(.5, 1., 0.);
//static int error_no = SHD_ERR_NOERR;
//static void set_error(int err);

Shader::Shader()
{
}

Shader::~Shader()
{
}

void Shader::Evaluate(const TraceContext &cxt,
    const SurfaceInput &in, SurfaceOutput *out) const
{
  evaluate(cxt, in, out);
}

int Shader::SetProperty(const std::string &prop_name, const PropertyValue &src_data)
{
  const Property *shd_props = get_property_list();
  const Property *dst_prop = PropFind(shd_props, src_data.type, prop_name.c_str());

  if (dst_prop == NULL) {
    return -1;
  }

  assert(dst_prop->SetProperty != NULL);
  // TODO is there better way to avoid this pointer?
  return dst_prop->SetProperty(this, &src_data);
}

/*
static void set_error(int err)
{
  error_no = err;
}
*/

} // namespace xxx
