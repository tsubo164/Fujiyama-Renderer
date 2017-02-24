// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_COMPATIBILITY_H
#define FJ_COMPATIBILITY_H

#if defined(_WIN32)
  #define FJ_WINDOWS
  #if defined(_WIN64)
    // 64bit Windows
    #include <stdint.h>
  #else
    // 32bit Windows
    #include <stdint.h>
  #endif
#elif defined(__APPLE__) || defined(MACOSX)
    // 64bit Mac OS X
    #define FJ_MACOSX
    #include <stdint.h>
#else
  #define FJ_LINUX
  #define __STDC_LIMIT_MACROS
  #if defined(__x86_64__)
    // 64bit Linux
    #include <stdint.h>
  #else
    // 32bit Linux
    #include <stdint.h>
  #endif
#endif

#if defined(FJ_WINDOWS)
  #define FJ_API_EXPORT __declspec(dllexport)
  #define FJ_API_IMPORT __declspec(dllimport)
#else
  #define FJ_API_EXPORT
  #define FJ_API_IMPORT
#endif

#if defined(FJ_DLL_EXPORT)
  #define FJ_API FJ_API_EXPORT
#else
  #define FJ_API FJ_API_IMPORT
#endif

// Plugins should always export their functions
#define FJ_PLUGIN_API FJ_API_EXPORT

#endif // FJ_XXX_H
