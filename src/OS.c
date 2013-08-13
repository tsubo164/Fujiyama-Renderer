/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "OS.h"
#include "Compatibility.h"

#if defined(SI_WINDOWS)
  #include "os_win.c"
#else
  #include "os_unix.c"
#endif
