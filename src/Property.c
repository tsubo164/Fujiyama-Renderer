/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Property.h"
#include <string.h>
#include <assert.h>

struct PropertyValue InitPropValue(void)
{
	const struct PropertyValue value = INIT_PROPERTYVALUE;
	return value;
}

const struct Property *PropFind(const struct Property *list, const char *name)
{
	const struct Property *p = list;
	const struct Property *found = NULL;

	while (p->name != NULL) {
		if (strcmp(p->name, name) == 0) {
			found = p;
			break;
		}
		p++;
	}
	return found;
}

