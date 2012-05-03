/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef BOX_H
#define BOX_H

#ifdef __cplusplus
extern "C" {
#endif

/* BOX2 */
#define BOX2_XSIZE(box) ((box)[2]-(box)[0])
#define BOX2_YSIZE(box) ((box)[3]-(box)[1])

#define BOX2_SET(dst,xmin,ymin,xmax,ymax) do { \
	(dst)[0] = (xmin); \
	(dst)[1] = (ymin); \
	(dst)[2] = (xmax); \
	(dst)[3] = (ymax); \
	} while(0)

#define BOX2_COPY(dst,a) do { \
	(dst)[0] = (a)[0]; \
	(dst)[1] = (a)[1]; \
	(dst)[2] = (a)[2]; \
	(dst)[3] = (a)[3]; \
	} while(0)

/* BOX3 */
#define BOX3_SET(dst,xmin,ymin,zmin,xmax,ymax,zmax) do { \
	(dst)[0] = (xmin); \
	(dst)[1] = (ymin); \
	(dst)[2] = (zmin); \
	(dst)[3] = (xmax); \
	(dst)[4] = (ymax); \
	(dst)[5] = (zmax); \
	} while(0)

#define BOX3_COPY(dst,a) do { \
	(dst)[0] = (a)[0]; \
	(dst)[1] = (a)[1]; \
	(dst)[2] = (a)[2]; \
	(dst)[3] = (a)[3]; \
	(dst)[4] = (a)[4]; \
	(dst)[5] = (a)[5]; \
	} while(0)

#define BOX3_EXPAND(dst,a) do { \
	(dst)[0] -= (a); \
	(dst)[1] -= (a); \
	(dst)[2] -= (a); \
	(dst)[3] += (a); \
	(dst)[4] += (a); \
	(dst)[5] += (a); \
	} while(0)

/* TODO TEST */
struct Box {
	double min[3];
	double max[3];
};

struct BoxHit {
	double tmin, tmax;
};

extern void BoxSet(struct Box *box,
		double xmin, double ymin, double zmin,
		double xmax, double ymax, double zmax);
extern void BoxExpand(struct Box *box, double delta);

extern void BoxPrint(const struct Box *box);

#if TEST
struct Ray;
extern int BoxRayIntersect(const struct Box *box, const struct Ray *ray,
		struct BoxHit *boxhit);
#endif


/* TODO TEST */
extern int BoxRayIntersect(const double *box,
		const double *rayorig, const double *raydir,
		double ray_tmin, double ray_tmax,
		double *hit_tmin, double *hit_tmax);

extern int BoxContainsPoint(const double *box, const double *point);

extern void BoxAddPoint(double *box, const double *point);

extern void BoxAddBox(double *box, const double *otherbox);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

