/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef INTERSECTION_H
#define INTERSECTION_H

#ifdef __cplusplus
extern "C" {
#endif

struct ObjectInstance;

struct Intersection {
	double P[3];
	double N[3];
	float Cd[3];
	float uv[2];

	double dPds[3];
	double dPdt[3];

	const struct ObjectInstance *object;
	int prim_id;

	double t_hit;
};

struct IntersectionList;

extern struct IntersectionList *IsectNew();
extern void IsectFree(struct IntersectionList *isects);

extern void IsectPush(struct IntersectionList *isects, const struct Intersection *isect);
extern const struct Intersection *IsectGet(struct IntersectionList *isects, int index);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

