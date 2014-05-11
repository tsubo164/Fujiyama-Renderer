/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_INTERVAL_H
#define FJ_INTERVAL_H

namespace fj {

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

} // namespace xxx

#endif /* FJ_XXX_H */
