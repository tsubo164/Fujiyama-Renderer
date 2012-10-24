/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "OS.h"
#include <stdio.h>
#include <stddef.h>

/* system dependent */
#include <dlfcn.h>
#include <sys/stat.h>

void *OsDlopen(const char *filename)
{
	void *handle = dlopen(filename, RTLD_LAZY);
	/* fputs(dlerror(), stderr); */
	return handle;
}

void *OsDlsym(void *handle, const char *symbol)
{
	void *sym = dlsym(handle, symbol);
	/* fputs(dlerror(), stderr); */
	return sym;
}

char *OsDlerror(void *handle)
{
	return dlerror();
}

int OsDlclose(void *handle)
{
	if (handle == NULL)
		return 0;

	/* dlclose() returns non-0 when error */
	if (dlclose(handle))
		return -1;
	else
		return 0;
}

