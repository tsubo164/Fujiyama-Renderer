/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef INTERVAL_H
#define INTERVAL_H

#ifdef __cplusplus
extern "C" {
#endif

struct IntervalList;
struct ObjectInstance;

/* ray-march interval for volumetric object */
struct Interval {
	double tmin;
	double tmax;
	const struct ObjectInstance *object;
	struct Interval *next;
};

extern struct IntervalList *IntervalListNew(void);
extern void IntervalListFree(struct IntervalList *intervals);

extern void IntervalListPush(struct IntervalList *intervals, const struct Interval *interval);
extern int IntervalListGetCount(const struct IntervalList *intervals);
extern double IntervalListGetMinT(const struct IntervalList *intervals);
extern double IntervalListGetMaxT(const struct IntervalList *intervals);
extern const struct Interval *IntervalListGetHead(const struct IntervalList *intervals);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

