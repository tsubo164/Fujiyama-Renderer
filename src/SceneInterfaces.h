/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef SCENEINTERFACES_H
#define SCENEINTERFACES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef long int ID;
typedef int Status;
enum { SI_BADID = -1 };
enum { SI_FAIL = -1, SI_SUCCESS = 0 };

/* Error interfaces */
extern int SiGetErrorNo(void);
extern const char *SiGetErrorMessage(int err_no);

/* Plugin interfaces */
extern Status SiOpenPlugin(const char *filename);

/* Scene interfaces */
extern Status SiOpenScene(void);
extern Status SiCloseScene(void);
extern Status SiRenderScene(ID renderer);
extern Status SiSaveFrameBuffer(ID framebuffer, const char *filename);

extern ID SiNewObjectInstance(ID accelerator);
extern ID SiNewFrameBuffer(const char *arg);
extern ID SiNewRenderer(void);
extern ID SiNewTexture(const char *arg);
extern ID SiNewCamera(const char *arg);
extern ID SiNewShader(const char *arg);
extern ID SiNewVolume(void);
extern ID SiNewCurve(const char *filename);
extern ID SiNewLight(const char *arg);
extern ID SiNewMesh(const char *filename);

extern Status SiAssignShader(ID object, ID shader);
extern Status SiAssignTexture(ID shader, const char *prop_name, ID texture);
extern Status SiAssignCamera(ID renderer, ID camera);
extern Status SiAssignFrameBuffer(ID renderer, ID framebuffer);

/* Property interfaces */
extern Status SiSetProperty1(ID id, const char *name, double v0);
extern Status SiSetProperty2(ID id, const char *name, double v0, double v1);
extern Status SiSetProperty3(ID id, const char *name, double v0, double v1, double v2);
extern Status SiSetProperty4(ID id, const char *name, double v0, double v1, double v2, double v3);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

