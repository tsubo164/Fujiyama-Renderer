/*
Copyright (c) 2011-2017 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_OS_H
#define FJ_OS_H

namespace fj {

extern void *OsDlopen(const char *filename);
extern void *OsDlsym(void *handle, const char *symbol);
extern char *OsDlerror(void *handle);
extern int OsDlclose(void *handle);

} // namespace xxx

#endif /* FJ_XXX_H */
