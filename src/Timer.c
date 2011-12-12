/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Timer.h"

void TimerStart(struct Timer *t)
{
	t->start = clock();
}

double TimerElapsed(const struct Timer *t)
{
	return (double)(clock() - t->start) / CLOCKS_PER_SEC;
}

