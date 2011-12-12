/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef CAMERA_H
#define CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

struct Camera;
struct Ray;

extern struct Camera *CamNew(const char *type);
extern void CamFree(struct Camera *cam);

extern void CamSetAspect(struct Camera *cam, double aspect);
extern void CamSetFov(struct Camera *cam, double fov);
extern void CamSetNearPlane(struct Camera *cam, double znear);
extern void CamSetFarPlane(struct Camera *cam, double zfar);
extern void CamSetPosition(struct Camera *cam, double xpos, double ypos, double zpos);
extern void CamSetDirection(struct Camera *cam, double xdir, double ydir, double zdir);

extern void CamGetRay(const struct Camera *cam, const double *screen_uv, struct Ray *ray);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

