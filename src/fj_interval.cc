// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_interval.h"
#include "fj_numeric.h"

namespace fj {

static void free_interval_nodes(Interval *head);
static int closer_than(const Interval *interval, const Interval *other);
static Interval *dup_interval(const Interval &src);
static void free_interval(Interval *interval);

IntervalList::IntervalList() :
    root_(),
    num_nodes_(0),
    tmin_(REAL_MAX),
    tmax_(-REAL_MAX)
{
}

IntervalList::~IntervalList()
{
  free_interval_nodes(root_.next);
}

void IntervalList::Push(const Interval &interval)
{
  Interval *new_node = dup_interval(interval);
  Interval *current = NULL;

  for (current = &root_; current != NULL; current = current->next) {
    if (current->next == NULL || closer_than(&interval, current->next)) {
      new_node->next = current->next;
      current->next = new_node;
      break;
    }
  }
  num_nodes_++;
  tmin_ = Min(tmin_, interval.tmin);
  tmax_ = Max(tmax_, interval.tmax);
}

int IntervalList::GetCount() const
{
  return num_nodes_;
}

Real IntervalList::GetMinT() const
{
  return tmin_;
}

Real IntervalList::GetMaxT() const
{
  return tmax_;
}

const Interval *IntervalList::GetHead() const
{
  return root_.next;
}

static void free_interval_nodes(Interval *head)
{
  Interval *current = head;

  while (current != NULL) {
    Interval *kill = current;
    Interval *next = current->next;
    free_interval(kill);
    current = next;
  }
}

static int closer_than(const Interval *interval, const Interval *other)
{
  if (interval->tmin < other->tmin)
    return 1;
  else
    return 0;
}

static Interval *dup_interval(const Interval &src)
{
  Interval *new_interval = new Interval(src);

  new_interval->next = NULL;

  return new_interval;
}

static void free_interval(Interval *interval)
{
  delete interval;
}

} // namespace xxx
