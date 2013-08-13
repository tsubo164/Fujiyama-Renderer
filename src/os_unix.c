/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include <stddef.h>
#include <dlfcn.h>
#include <sys/stat.h>

void *OsDlopen(const char *filename)
{
  void *handle = dlopen(filename, RTLD_LAZY);
  return handle;
}

void *OsDlsym(void *handle, const char *symbol)
{
  void *sym = dlsym(handle, symbol);
  return sym;
}

char *OsDlerror(void *handle)
{
  return dlerror();
}

int OsDlclose(void *handle)
{
  int err = 0;

  if (handle == NULL) {
    return 0;
  }

  err = dlclose(handle);
  if (err) {
    return -1;
  } else {
    return 0;
  }
}
