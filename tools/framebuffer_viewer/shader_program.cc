// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "shader_program.h"
#include <iostream>

ShaderProgram::ShaderProgram() :
    vert_shader_id_(0),
    frag_shader_id_(0),
    program_id_(0)
{
}

ShaderProgram::~ShaderProgram()
{
}

int ShaderProgram::Init()
{
  static const GLchar *vert_source[] = {
  "#version 120\n"
  "void main(void)\n"
  "{\n"
  "  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
  "  gl_Position = ftransform();\n"
  "}\n"
  };
  static const GLchar *source[] = {
  "#version 120\n"
  "uniform sampler2D texture;\n"
  "uniform int display_channels;\n"
  "uniform int channel_count;\n"
  "void main(void)\n"
  "{\n"
  "  vec4 C_tex = texture2DProj(texture, gl_TexCoord[0]);\n"
  "  if (channel_count == 1) {\n"
  "    C_tex[1] = C_tex[0];\n"
  "    C_tex[2] = C_tex[0];\n"
  "  }\n"
  "  if (display_channels != -1) {\n"
  "    float C_disp = C_tex[display_channels];\n"
  "    C_tex[0] = C_disp;\n"
  "    C_tex[1] = C_disp;\n"
  "    C_tex[2] = C_disp;\n"
  "    C_tex[3] = 1.;\n"
  "  }\n"
  "  gl_FragColor = C_tex;\n"
  "}\n"
  };
  GLuint vert_shader_id = 0;
  GLuint frag_shader_id = 0;
  GLuint program_id = 0;

  GLint compiled = GL_FALSE;
  GLint linked = GL_FALSE;

  vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_shader_id, 1, vert_source, NULL);
  glCompileShader(vert_shader_id);
  glGetShaderiv(vert_shader_id, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_FALSE) {
    std::cerr << "vertex shader compile error\n";
  }

  frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_shader_id, 1, source, NULL);
  glCompileShader(frag_shader_id);
  glGetShaderiv(frag_shader_id, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_FALSE) {
    std::cerr << "fragment shader compile error\n";
  }

  program_id = glCreateProgram();
  glAttachShader(program_id, vert_shader_id);
  glAttachShader(program_id, frag_shader_id);

  glLinkProgram(program_id);
  glGetProgramiv(program_id, GL_LINK_STATUS, &linked);
  if (linked == GL_FALSE) {
    std::cerr << "shader program link error\n";
  }

  vert_shader_id_ = vert_shader_id;
  frag_shader_id_ = frag_shader_id;
  program_id_ = program_id;

  return 0;
}

void ShaderProgram::SetUniformInt(const char *variable_name, GLint value) const
{
  glUniform1i(glGetUniformLocation(program_id_, variable_name), value);
}

GLuint ShaderProgram::GetVertexShaderID() const
{
  return vert_shader_id_;
}

GLuint ShaderProgram::GetFragmentShaderID() const
{
  return frag_shader_id_;
}

GLuint ShaderProgram::GetProgramID() const
{
  return program_id_;
}
