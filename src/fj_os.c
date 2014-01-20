/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_os.h"
#include "fj_compatibility.h"

#if defined(FJ_WINDOWS)
  #include "internal/fj_os_win.c"
#elif defined(FJ_LINUX)
  #include "internal/fj_os_unix.c"
#elif defined(FJ_MACOSX)
  #include "internal/fj_os_mac.c"
#else
#endif
