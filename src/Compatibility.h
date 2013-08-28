/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

#if defined(_WIN32)
  #define SI_WINDOWS
  #if defined(_WIN64)
    /* 64bit Windows */
    typedef unsigned int uint32_t;
    #define UINT32_MAX UINT_MAX
  #else
    /* 32bit Windows */
    typedef unsigned int uint32_t;
    #define UINT32_MAX UINT_MAX
  #endif
#elif defined(__APPLE__) || defined(MACOSX)
    /* 64bit Mac OS X */
    #define SI_MACOSX
    typedef unsigned int uint32_t;
    #define UINT32_MAX UINT_MAX
#else
  #define SI_LINUX
  #if defined(__x86_64__)
    /* 64bit Linux */
    typedef unsigned int uint32_t;
    #define UINT32_MAX UINT_MAX
  #else
    /* 32bit Linux */
    typedef unsigned int uint32_t
    #define UINT32_MAX UINT_MAX
  #endif
#endif

/* -- SI_EXTERN_C -- */
#if defined(__cplusplus)
  #define SI_EXTERN_C extern "C"
#else
  #define SI_EXTERN_C
#endif

/* -- SI_EXPORT -- */
#if defined(SI_WINDOWS)
  /* DLL projects should define CORELIBRARY_EXPORTS */
  #if defined(CORELIBRARY_EXPORTS)
    #define SI_EXPORT SI_EXTERN_C __declspec(dllexport)
  #else
    #define SI_EXPORT SI_EXTERN_C __declspec(dllimport)
  #endif
#else
    #define SI_EXPORT
#endif

#endif /* XXX_H */
