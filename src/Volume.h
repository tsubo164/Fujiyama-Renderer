/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef VOLUME_H
#define VOLUME_H

#ifdef __cplusplus
extern "C" {
#endif

struct Volume;

struct VolumeSample {
	float density;
};

extern struct Volume *VolNew(void);
extern void VolFree(struct Volume *volume);

extern void VolResize(struct Volume *volume, int xres, int yres, int zres);
extern void VolSetBounds(struct Volume *volume, double *bounds);
extern void VolGetBounds(const struct Volume *volume, double *bounds);
extern void VolGetResolution(const struct Volume *volume, int *i, int *j, int *k);

extern double VolGetFilterSize(const struct Volume *volume);

extern void VolIndexToPoint(const struct Volume *volume, int i, int j, int k, double *point);
extern void VolPointToIndex(const struct Volume *volume, const double *point,
		int *i, int *j, int *k);

extern void VolGetIndexRange(const struct Volume *volume,
		const double *center, double radius,
		int *xmin, int *ymin, int *zmin,
		int *xmax, int *ymax, int *zmax);

extern void VolSetValue(struct Volume *volume, int x, int y, int z, float value);
extern float VolGetValue(const struct Volume *volume, int x, int y, int z);

extern int VolGetSample(const struct Volume *volume, const double *point,
			struct VolumeSample *sample);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

