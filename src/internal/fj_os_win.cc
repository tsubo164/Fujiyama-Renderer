// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

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
  sym = GetProcAddress(reinterpret_cast<HMODULE>(handle), symbol);

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

  return reinterpret_cast<char *>(lpMsgBuf);
}

int OsDlclose(void *handle)
{
  BOOL ret = 0;

  if (handle == NULL) {
    return 0;
  }

  SetLastError(NO_ERROR);
  ret = FreeLibrary(reinterpret_cast<HMODULE>(handle));

  if (ret == 0) {
    return -1;
  } else {
    return 0;
  }
}
