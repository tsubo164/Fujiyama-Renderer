/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
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

extern void VolGetBounds(const struct Volume *volume, double *bounds);
extern int VolGetSample(const struct Volume *volume, const double *point,
			struct VolumeSample *sample);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

