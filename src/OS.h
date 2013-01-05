/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef OS_H
#define OS_H

#ifdef __cplusplus
extern "C" {
#endif

void *OsDlopen(const char *filename);
void *OsDlsym(void *handle, const char *symbol);
char *OsDlerror(void *handle);
int OsDlclose(void *handle);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

