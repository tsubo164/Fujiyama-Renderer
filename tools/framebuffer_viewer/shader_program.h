// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef GLSL_SHADERS_H
#define GLSL_SHADERS_H

#include "compatible_opengl.h"

class ShaderProgram {
public:
  ShaderProgram();
  ~ShaderProgram();

  int Init();

  void SetUniformInt(const char *variable_name, GLint value) const;

  GLuint GetVertexShaderID() const;
  GLuint GetFragmentShaderID() const;
  GLuint GetProgramID() const;

private:
  GLuint vert_shader_id_;
  GLuint frag_shader_id_;
  GLuint program_id_;
};

#endif // XXX_H
