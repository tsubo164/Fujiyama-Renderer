/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "OS.h"
#include "Compatibility.h"

#if defined(SI_WINDOWS)
  #include "internal/fj_os_win.c"
#else
  #include "internal/fj_os_unix.c"
#endif
