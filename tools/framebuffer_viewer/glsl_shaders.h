// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef GLSL_SHADERS_H
#define GLSL_SHADERS_H

#include "compatible_opengl.h"

class ShaderProgram {
public:
  ShaderProgram() :
      vert_shader_id(0),
      frag_shader_id(0),
      program_id(0) {}
  ~ShaderProgram() {}

public:
  GLuint vert_shader_id;
  GLuint frag_shader_id;
  GLuint program_id;
};

extern int init_shaders(ShaderProgram *prog);

extern void set_uniform_int(const ShaderProgram *prog,
    const char *variable_name, GLint value);

#endif // XXX_H
