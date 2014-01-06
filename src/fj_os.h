/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_OS_H
#define FJ_OS_H

#ifdef __cplusplus
extern "C" {
#endif

extern void *OsDlopen(const char *filename);
extern void *OsDlsym(void *handle, const char *symbol);
extern char *OsDlerror(void *handle);
extern int OsDlclose(void *handle);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
