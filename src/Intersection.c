/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "LocalGeometry.h"
#include "Array.h"
#include <stdlib.h>
#include <assert.h>

struct IntersectionList {
	struct Array *intersections;
};

struct IntersectionList *IsectNew()
{
	struct IntersectionList *list;

	list = (struct IntersectionList *) malloc(sizeof(struct IntersectionList));
	if (list == NULL)
		return NULL;

	list->intersections = ArrNew(sizeof(struct Intersection));
	if (list->intersections == NULL)
		return NULL;

	return list;
}

void IsectFree(struct IntersectionList *list)
{
	if (list == NULL)
		return;
	
	ArrFree(list->intersections);
	free(list);
}

