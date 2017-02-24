// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/stat.h>

void *OsDlopen(const char *filename)
{
  static char filename_with_ext[1024] = {'\0'};
  const char *input_ext = strrchr(filename, '.');
  const char *added_ext = "";
  void *handle = NULL;

  if (input_ext == NULL || strcmp(input_ext, ".so") != 0) {
    added_ext = ".so";
  }
  sprintf(filename_with_ext, "%s%s", filename, added_ext);

  handle = dlopen(filename_with_ext, RTLD_LAZY);
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
