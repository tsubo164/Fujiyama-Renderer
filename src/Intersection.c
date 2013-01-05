/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Intersection.h"
#include "Array.h"
#include <stdlib.h>
#include <assert.h>

struct IntersectionList {
	struct Array *list;
};

struct IntersectionList *IsectNew()
{
	struct IntersectionList *isects;

	isects = (struct IntersectionList *) malloc(sizeof(struct IntersectionList));
	if (isects == NULL)
		return NULL;

	isects->list = ArrNew(sizeof(struct Intersection));
	if (isects->list == NULL)
		return NULL;

	return isects;
}

void IsectFree(struct IntersectionList *isects)
{
	if (isects == NULL)
		return;
	
	ArrFree(isects->list);
	free(isects);
}

void IsectPush(struct IntersectionList *isects, const struct Intersection *isect)
{
	ArrPush(isects->list, isect);
}

