/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include <stddef.h>
#include <windows.h>

void *OsDlopen(const char *filename)
{
  void *handle = NULL;

  SetLastError(NO_ERROR);
  handle = LoadLibrary(filename);

  return handle;
}

void *OsDlsym(void *handle, const char *symbol)
{
  void *sym = NULL;

  SetLastError(NO_ERROR);
  sym = GetProcAddress(handle, symbol);

  return sym;
}

char *OsDlerror(void *handle)
{
  LPVOID lpMsgBuf;

  FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      GetLastError(),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &lpMsgBuf,
      0,
      NULL
  );

  return lpMsgBuf;
}

int OsDlclose(void *handle)
{
  BOOL ret = 0;

  if (handle == NULL) {
    return 0;
  }

  SetLastError(NO_ERROR);
  ret = FreeLibrary(handle);

  if (ret == 0) {
    return -1;
  } else {
    return 0;
  }
}
