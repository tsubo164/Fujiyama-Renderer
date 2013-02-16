/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VEC_OP_INLINE 1
#if VEC_OP_INLINE
#define VecMulAssign(dst,value) do { \
	(dst)[0] *= (value); \
	(dst)[1] *= (value); \
	(dst)[2] *= (value); \
	} while(0)
#else
void VecMulAssign(struct Vector *dst, double value);
#endif

/* VEC2 ARRAY */
#define VEC2_REALLOC(ptr,type,nelems) ((type*)realloc((ptr), sizeof(type)*2*(nelems)))
#define VEC2_ALLOC(type,nelems) ((type*)malloc(sizeof(type)*2*(nelems)))
#define VEC2_NTH(ptr,index) ((ptr)+2*(index))

/* VEC3 ARRAY */
#define VEC3_REALLOC(ptr,type,nelems) ((type*)realloc((ptr), sizeof(type)*3*(nelems)))
#define VEC3_ALLOC(type,nelems) ((type*)malloc(sizeof(type)*3*(nelems)))
#define VEC3_NTH(ptr,index) ((ptr)+3*(index))

/* VEC2 */
#define VEC2_SET(dst,x,y) do { \
	(dst)[0] = (x); \
	(dst)[1] = (y); \
	} while(0)

#define VEC2_COPY(dst,a) do { \
	(dst)[0] = (a)[0]; \
	(dst)[1] = (a)[1]; \
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

#define VEC3_LERP(dst,a,b,t) do { \
	(dst)[0] = (1-(t)) * (a)[0] + (t) * (b)[0]; \
	(dst)[1] = (1-(t)) * (a)[1] + (t) * (b)[1]; \
	(dst)[2] = (1-(t)) * (a)[2] + (t) * (b)[2]; \
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

#define VEC4_LERP(dst,a,b,t) do { \
	(dst)[0] = (1-(t)) * (a)[0] + (t) * (b)[0]; \
	(dst)[1] = (1-(t)) * (a)[1] + (t) * (b)[1]; \
	(dst)[2] = (1-(t)) * (a)[2] + (t) * (b)[2]; \
	(dst)[3] = (1-(t)) * (a)[3] + (t) * (b)[3]; \
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

