/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "OS.h"
#include <stddef.h>

/* system dependent */
#include <dlfcn.h>
#include <sys/stat.h>

void *OsDlopen(const char *filename)
{
	return dlopen(filename, RTLD_LAZY);
}

void *OsDlsym(void *handle, const char *symbol)
{
	return dlsym(handle, symbol);
}

char *OsDlerror(void *handle)
{
	return dlerror();
}

int OsDlclose(void *handle)
{
	/* dlclose() returns non-0 when error */
	if (dlclose(handle))
		return -1;
	else
		return 0;
}

