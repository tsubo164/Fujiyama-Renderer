/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef RAY_H
#define RAY_H

#ifdef __cplusplus
extern "C" {
#endif

#define POINT_ON_RAY(dst,orig,dir,t) do { \
	(dst)[0] = (orig)[0] + (t) * (dir)[0]; \
	(dst)[1] = (orig)[1] + (t) * (dir)[1]; \
	(dst)[2] = (orig)[2] + (t) * (dir)[2]; \
	} while (0)

struct Ray {
	double orig[3];
	double dir[3];

	double tmin;
	double tmax;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

