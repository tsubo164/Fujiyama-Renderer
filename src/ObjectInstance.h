/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef OBJECTINSTANCE_H
#define OBJECTINSTANCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Transform.h"

struct Intersection;
struct VolumeSample;
struct Accelerator;
struct Shader;
struct Volume;
struct Light;
struct Ray;

struct ObjectInstance;
struct ObjectGroup;

struct ObjectContent {
	const struct Accelerator *surface;
	const struct Volume *volume;
};

/* ObjectInstance interfaces */
extern struct ObjectInstance *ObjNew(const struct Accelerator *acc);
extern void ObjFree(struct ObjectInstance *obj);

/* TODO TEST remove ObjSet*** if possible */
extern void ObjSetSurface(struct ObjectInstance *obj, const struct Accelerator *acc);
extern int ObjSetVolume(struct ObjectInstance *obj, const struct Volume *volume);
extern int ObjIsSurface(const struct ObjectInstance *obj);
extern int ObjIsVolume(const struct ObjectInstance *obj);

extern void ObjSetTranslate(struct ObjectInstance *obj, double tx, double ty, double tz);
extern void ObjSetRotate(struct ObjectInstance *obj, double rx, double ry, double rz);
extern void ObjSetScale(struct ObjectInstance *obj, double sx, double sy, double sz);
extern void ObjSetTransformOrder(struct ObjectInstance *obj, int order);
extern void ObjSetRotateOrder(struct ObjectInstance *obj, int order);
extern void ObjSetShader(struct ObjectInstance *obj, const struct Shader *shader);
extern void ObjSetLightList(struct ObjectInstance *obj, const struct Light **lights, int count);
extern void ObjSetReflectTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp);
extern void ObjSetRefractTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp);

extern const struct ObjectGroup *ObjGetReflectTarget(const struct ObjectInstance *obj);
extern const struct ObjectGroup *ObjGetRefractTarget(const struct ObjectInstance *obj);
extern const struct Shader *ObjGetShader(const struct ObjectInstance *obj);
extern const struct Light **ObjGetLightList(const struct ObjectInstance *obj);
extern int ObjGetLightCount(const struct ObjectInstance *obj);
extern void ObjGetBounds(const struct ObjectInstance *obj, double *bounds);

extern int ObjIntersect(const struct ObjectInstance *obj, const struct Ray *ray,
			struct Intersection *isect);
extern int ObjGetVolumeSample(const struct ObjectInstance *obj, const double *point,
			struct VolumeSample *sample);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

