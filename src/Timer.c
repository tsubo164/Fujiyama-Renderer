/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Timer.h"

void TimerStart(struct Timer *t)
{
	t->start = clock();
}

struct Elapse TimerElapsed(const struct Timer *t)
{
	const double seconds = (double)(clock() - t->start) / CLOCKS_PER_SEC;
	struct Elapse elapse;

	elapse.hour = (int) (seconds / (60*60));
	elapse.min = (int) ((seconds - elapse.hour*60*60) / 60);
	elapse.sec = seconds - elapse.hour*60*60 - elapse.min*60;

	return elapse;
}

