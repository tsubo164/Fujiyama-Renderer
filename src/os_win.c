/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include <stddef.h>
#include <windows.h>

void *OsDlopen(const char *filename)
{
  void *handle = LoadLibrary(filename);
  return handle;
}

void *OsDlsym(void *handle, const char *symbol)
{
  void *sym = GetProcAddress(handle, symbol);
  return sym;
}

char *OsDlerror(void *handle)
{
  return NULL;
  /*
  return dlerror();
  */
}

int OsDlclose(void *handle)
{
  BOOL ret = 0;

  if (handle == NULL) {
    return 0;
  }

  ret = FreeLibrary(handle);

  if (ret == 0) {
    return -1;
  } else {
    return 0;
  }
}
