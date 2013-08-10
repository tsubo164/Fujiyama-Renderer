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

/* SI_EXPORT */
#if defined(SI_WINDOWS)
  /* DLL projects should define _EXPORT */
  #if defined(_EXPORT)
    #define SI_EXPORT extern __declspec(dllexport)
  #else
    #define SI_EXPORT extern __declspec(dllimport)
  #endif
#else
    #define SI_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */
