/*
Copyright (c) 2011-2016 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_os.h"
#include "fj_compatibility.h"

namespace fj {

#if defined(FJ_WINDOWS)
  #include "internal/fj_os_win.cc"
#elif defined(FJ_LINUX)
  #include "internal/fj_os_unix.cc"
#elif defined(FJ_MACOSX)
  #include "internal/fj_os_mac.cc"
#else
#endif

} // namespace xxx
