/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef GLSL_SHADERS_H
#define GLSL_SHADERS_H

#include "compatible_opengl.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ShaderProgram {
  GLuint vert_shader_id;
  GLuint frag_shader_id;
  GLuint program_id;
};

extern int init_shaders(struct ShaderProgram *prog);

extern void set_uniform_int(const struct ShaderProgram *prog,
    const char *variable_name, GLint value);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */
