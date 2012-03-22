/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef LOCALGEOMETRY_H
#define LOCALGEOMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

struct ObjectInstance;

struct LocalGeometry {
	double P[3];
	double N[3];
	float Cd[3];
	float uv[2];

	double dPds[3];
	double dPdt[3];

	const struct ObjectInstance *object;
	int prim_id;
};

struct Intersection {
	struct LocalGeometry geo;
	double hit_t;
};

struct IntersectionList;

extern struct IntersectionList *IsectNew();
extern void IsectFree(struct IntersectionList *list);

extern int IsectPush(struct IntersectionList *list, const struct LocalGeometry *local);
extern const struct LocalGeometry *IsectGet(struct IntersectionList *list, int index);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

