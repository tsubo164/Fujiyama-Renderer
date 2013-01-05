/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Interval.h"
#include "Numeric.h"
#include <stdlib.h>
#include <float.h>

struct IntervalList {
	struct Interval root;
	int num_nodes;
	double tmin;
	double tmax;
};

static void free_interval_nodes(struct Interval *head);
static int closer_than(const struct Interval *interval, const struct Interval *other);
static struct Interval *dup_interval(const struct Interval *src);
static void free_interval(struct Interval *interval);

struct IntervalList *IntervalListNew(void)
{
	struct IntervalList *intervals;

	intervals = (struct IntervalList *) malloc(sizeof(struct IntervalList));
	if (intervals == NULL)
		return NULL;

	intervals->root.tmin = -FLT_MAX;
	intervals->root.tmax = FLT_MAX;
	intervals->root.object = NULL;
	intervals->root.next = NULL;

	intervals->num_nodes = 0;
	intervals->tmin = FLT_MAX;
	intervals->tmax = -FLT_MAX;

	return intervals;
}

void IntervalListFree(struct IntervalList *intervals)
{
	if (intervals == NULL)
		return;

	free_interval_nodes(intervals->root.next);
	free(intervals);
}

void IntervalListPush(struct IntervalList *intervals, const struct Interval *interval)
{
	struct Interval *new_node;
	struct Interval *current;

	new_node = dup_interval(interval);
	if (new_node == NULL) {
		/* TODO error handling */
		return;
	}

	for (current = &intervals->root; current != NULL; current = current->next) {
		if (closer_than(interval, current->next)) {
			new_node->next = current->next;
			current->next = new_node;
			break;
		}
	}
	intervals->num_nodes++;
	intervals->tmin = MIN(intervals->tmin, interval->tmin);
	intervals->tmax = MAX(intervals->tmax, interval->tmax);
}

int IntervalListGetCount(const struct IntervalList *intervals)
{
	return intervals->num_nodes;
}

double IntervalListGetMinT(const struct IntervalList *intervals)
{
	return intervals->tmin;
}

double IntervalListGetMaxT(const struct IntervalList *intervals)
{
	return intervals->tmax;
}

const struct Interval *IntervalListGetHead(const struct IntervalList *intervals)
{
	return intervals->root.next;
}

static void free_interval_nodes(struct Interval *head)
{
	struct Interval *current = head;

	while (current != NULL) {
		struct Interval *kill = current;
		struct Interval *next = current->next;
		free_interval(kill);
		current = next;
	}
}

static int closer_than(const struct Interval *interval, const struct Interval *other)
{
	if (other == NULL || interval->tmin < other->tmin)
		return 1;
	else
		return 0;
}

static struct Interval *dup_interval(const struct Interval *src)
{
	struct Interval *new_node;

	new_node = (struct Interval *) malloc(sizeof(struct Interval));
	if (new_node == NULL)
		return NULL;

	*new_node = *src;
	new_node->next = NULL;

	return new_node;
}

static void free_interval(struct Interval *interval)
{
	if (interval == NULL)
		return;

	free(interval);
}

