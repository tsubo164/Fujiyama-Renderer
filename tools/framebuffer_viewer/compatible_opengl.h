/*
Copyright (c) 2011-2020 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef COMPATIBLEOPENGL_H
#define COMPATIBLEOPENGL_H

#include "fj_compatibility.h"

#if defined(FJ_WINDOWS)
  #pragma comment(lib, "glew32.lib")
  #pragma comment(lib, "freeglut.lib")
  #include <GL/glew.h>
  #include <GL/glut.h>
#elif defined(FJ_MACOSX)
  #include <GLUT/glut.h>
#else
  #define GL_GLEXT_PROTOTYPES
  #include <GL/glut.h>
#endif

#endif /* XXX_H */
