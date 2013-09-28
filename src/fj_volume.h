/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_VOLUME_H
#define FJ_VOLUME_H

#ifdef __cplusplus
extern "C" {
#endif

struct Volume;
struct Vector;
struct Box;

struct VolumeSample {
  float density;
};

extern struct Volume *VolNew(void);
extern void VolFree(struct Volume *volume);

extern void VolResize(struct Volume *volume, int xres, int yres, int zres);
extern void VolSetBounds(struct Volume *volume, struct Box *bounds);
extern void VolGetBounds(const struct Volume *volume, struct Box *bounds);
extern void VolGetResolution(const struct Volume *volume, int *i, int *j, int *k);

extern double VolGetFilterSize(const struct Volume *volume);

extern void VolIndexToPoint(const struct Volume *volume, int i, int j, int k,
    struct Vector *point);
extern void VolPointToIndex(const struct Volume *volume, const struct Vector *point,
    int *i, int *j, int *k);

extern void VolGetIndexRange(const struct Volume *volume,
    const struct Vector *center, double radius,
    int *xmin, int *ymin, int *zmin,
    int *xmax, int *ymax, int *zmax);

extern void VolSetValue(struct Volume *volume, int x, int y, int z, float value);
extern float VolGetValue(const struct Volume *volume, int x, int y, int z);

extern int VolGetSample(const struct Volume *volume, const struct Vector *point,
      struct VolumeSample *sample);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */

