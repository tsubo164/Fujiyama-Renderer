/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef BOX_H
#define BOX_H

#include "Vector.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Box {
	struct Vector min;
	struct Vector max;
};

/* TODO delete BOX2 */
/* BOX2 */
/* BOX2[4] {min{0, 0}, max{0, 0}} */
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
/* double box3[6] {min{0, 0, 0}, max{0, 0, 0}} */
#define BOX3_XSIZE(box) ((box)->max.x - (box)->min.x)
#define BOX3_YSIZE(box) ((box)->max.y - (box)->min.y)
#define BOX3_ZSIZE(box) ((box)->max.z - (box)->min.z)

#define BOX3_SET(dst,xmin,ymin,zmin,xmax,ymax,zmax) do { \
	(dst)->min.x = (xmin); \
	(dst)->min.y = (ymin); \
	(dst)->min.z = (zmin); \
	(dst)->max.x = (xmax); \
	(dst)->max.y = (ymax); \
	(dst)->max.z = (zmax); \
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
	(dst)->min.x -= (a); \
	(dst)->min.y -= (a); \
	(dst)->min.z -= (a); \
	(dst)->max.x += (a); \
	(dst)->max.y += (a); \
	(dst)->max.z += (a); \
	} while(0)

extern int BoxContainsPoint(const struct Box *box, const struct Vector *point);
extern void BoxAddPoint(struct Box *box, const struct Vector *point);
extern void BoxAddBox(struct Box *box, const struct Box *otherbox);

extern int BoxRayIntersect(const struct Box *box,
		const struct Vector *rayorig, const struct Vector *raydir,
		double ray_tmin, double ray_tmax,
		double *hit_tmin, double *hit_tmax);

extern void BoxCentroid(const struct Box *box, struct Vector *centroid);
extern double BoxDiagonal(const struct Box *box);

extern void BoxPrint(const struct Box *box);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

