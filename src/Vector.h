/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef VECTOR_H
#define VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

/* VEC2 */
#define VEC2_SET(dst,x,y) do { \
	(dst)[0] = (x); \
	(dst)[1] = (y); \
	} while(0)

#define VEC2_DOT(a,b) ((a)[0] * (b)[0] + (a)[1] * (b)[1])

/* VEC3 */
#define VEC3_SET(dst,x,y,z) do { \
	(dst)[0] = (x); \
	(dst)[1] = (y); \
	(dst)[2] = (z); \
	} while(0)

#define VEC3_GET(x,y,z,src) do { \
	(x) = (src)[0]; \
	(y) = (src)[1]; \
	(z) = (src)[2]; \
	} while(0)

#define VEC3_COPY(dst,a) do { \
	(dst)[0] = (a)[0]; \
	(dst)[1] = (a)[1]; \
	(dst)[2] = (a)[2]; \
	} while(0)

#define VEC3_ADD(dst,a,b) do { \
	(dst)[0] = (a)[0] + (b)[0]; \
	(dst)[1] = (a)[1] + (b)[1]; \
	(dst)[2] = (a)[2] + (b)[2]; \
	} while(0)

#define VEC3_ADD_ASGN(dst,a) do { \
	(dst)[0] += (a)[0]; \
	(dst)[1] += (a)[1]; \
	(dst)[2] += (a)[2]; \
	} while(0)

#define VEC3_SUB(dst,a,b) do { \
	(dst)[0] = (a)[0] - (b)[0]; \
	(dst)[1] = (a)[1] - (b)[1]; \
	(dst)[2] = (a)[2] - (b)[2]; \
	} while(0)

#define VEC3_MUL(dst,a,val) do { \
	(dst)[0] = (a)[0] * val; \
	(dst)[1] = (a)[1] * val; \
	(dst)[2] = (a)[2] * val; \
	} while(0)

#define VEC3_MUL_ASGN(a,val) do { \
	(a)[0] *= val; \
	(a)[1] *= val; \
	(a)[2] *= val; \
	} while(0)

/* no check for zero division */
#define VEC3_DIV_ASGN(a,val) do { \
	double inv = 1. / val; \
	VEC3_MUL_ASGN((a),inv); \
	} while(0)

#define VEC3_DOT(a,b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])

#define VEC3_LEN(a) (sqrt(VEC3_DOT((a),(a))))

#define VEC3_NORMALIZE(a) do { \
	double len = VEC3_LEN((a)); \
	if (len == 0) break; \
	VEC3_DIV_ASGN((a),len); \
	} while(0)

#define VEC3_CROSS(dst,a,b) do { \
	(dst)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1]; \
	(dst)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2]; \
	(dst)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0]; \
	} while(0)

/* VEC4 */
#define VEC4_SET(dst,x,y,z,w) do { \
	(dst)[0] = (x); \
	(dst)[1] = (y); \
	(dst)[2] = (z); \
	(dst)[3] = (w); \
	} while(0)

#define VEC4_COPY(dst,a) do { \
	(dst)[0] = (a)[0]; \
	(dst)[1] = (a)[1]; \
	(dst)[2] = (a)[2]; \
	(dst)[3] = (a)[3]; \
	} while(0)

#define VEC4_ADD(dst,a,b) do { \
	(dst)[0] = (a)[0] + (b)[0]; \
	(dst)[1] = (a)[1] + (b)[1]; \
	(dst)[2] = (a)[2] + (b)[2]; \
	(dst)[3] = (a)[3] + (b)[3]; \
	} while(0)

#define VEC4_SUB(dst,a,b) do { \
	(dst)[0] = (a)[0] - (b)[0]; \
	(dst)[1] = (a)[1] - (b)[1]; \
	(dst)[2] = (a)[2] - (b)[2]; \
	(dst)[3] = (a)[3] - (b)[3]; \
	} while(0)

#define VEC4_MUL(a,val) do { \
	(a)[0] *= val; \
	(a)[1] *= val; \
	(a)[2] *= val; \
	(a)[3] *= val; \
	} while(0)

/* no check for zero division */
#define VEC4_DIV(a,val) do { \
	double inv = 1. / val; \
	VEC4_MUL((a),inv); \
	} while(0)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

