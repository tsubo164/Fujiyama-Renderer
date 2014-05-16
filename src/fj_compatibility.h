/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_COMPATIBILITY_H
#define FJ_COMPATIBILITY_H

#if defined(_WIN32)
  #define FJ_WINDOWS
  #if defined(_WIN64)
    /* 64bit Windows */
    #include <stdint.h>
  #else
    /* 32bit Windows */
    #include <stdint.h>
  #endif
#elif defined(__APPLE__) || defined(MACOSX)
    /* 64bit Mac OS X */
    #define FJ_MACOSX
    #include <stdint.h>
#else
  #define FJ_LINUX
  #if defined(__x86_64__)
    /* 64bit Linux */
    #include <stdint.h>
  #else
    /* 32bit Linux */
    #include <stdint.h>
  #endif
#endif

#if defined(FJ_WINDOWS)
  #define FJ_API __declspec(dllexport)
#else
  #define FJ_API
#endif

#endif /* FJ_XXX_H */
