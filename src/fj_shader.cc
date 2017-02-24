// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"

namespace fj {

static const Color NO_SHADER_COLOR(.5, 1., 0.);

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

} // namespace xxx
