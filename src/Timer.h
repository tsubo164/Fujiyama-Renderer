/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

struct Timer {
	clock_t start;
};

extern void TimerStart(struct Timer *t);
extern double TimerElapsed(const struct Timer *t);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

