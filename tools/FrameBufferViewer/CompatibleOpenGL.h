/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef COMPATIBLEOPENGL_H
#define COMPATIBLEOPENGL_H

#include "Compatibility.h"

#if defined(SI_WINDOWS)
#  pragma comment(lib, "glew32.lib")
#  include "glew.h"
#  include "glut.h"
#  include "glext.h"
#elif defined(SI_MACOSX)
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif

#endif /* XXX_H */
