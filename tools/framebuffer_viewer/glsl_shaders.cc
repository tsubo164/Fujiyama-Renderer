// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "glsl_shaders.h"
#include <cstdio>

int init_shaders(ShaderProgram *prog)
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
    fprintf(stderr, "vertex shader compile error\n");
  }

  frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_shader_id, 1, source, NULL);
  glCompileShader(frag_shader_id);
  glGetShaderiv(frag_shader_id, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_FALSE) {
    fprintf(stderr, "fragment shader compile error\n");
  }

  program_id = glCreateProgram();
  glAttachShader(program_id, vert_shader_id);
  glAttachShader(program_id, frag_shader_id);

  glLinkProgram(program_id);
  glGetProgramiv(program_id, GL_LINK_STATUS, &linked);
  if (linked == GL_FALSE) {
    fprintf(stderr, "link error\n");
  }

  prog->vert_shader_id = vert_shader_id;
  prog->frag_shader_id = frag_shader_id;
  prog->program_id = program_id;

  return 0;
}

void set_uniform_int(const ShaderProgram *prog,
    const char *variable_name, GLint value)
{
    glUniform1i(glGetUniformLocation(prog->program_id, variable_name), value);
}
