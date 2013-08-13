/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef OS_H
#define OS_H

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

#endif /* XXX_H */
