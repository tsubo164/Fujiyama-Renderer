/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef SCENE_H
#define SCENE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ObjectInstance.h"
#include "Accelerator.h"
#include "FrameBuffer.h"
#include "Renderer.h"
#include "Texture.h"
#include "Camera.h"
#include "Plugin.h"
#include "Shader.h"
#include "Volume.h"
#include "Curve.h"
#include "Light.h"
#include "Mesh.h"

struct Scene;

/* Scene */
extern struct Scene *ScnNew(void);
extern void ScnFree(struct Scene *scn);

/* ObjectInstance */
extern struct ObjectInstance *ScnNewObjectInstance(struct Scene *scn, const struct Accelerator *acc);
extern struct ObjectInstance **ScnGetObjectInstanceList(const struct Scene *scn);
extern struct ObjectInstance *ScnGetObjectInstance(const struct Scene *scn, int index);
extern size_t ScnGetObjectInstanceCount(const struct Scene *scn);

/* Accelerator */
extern struct Accelerator *ScnNewAccelerator(struct Scene *scn, int accelerator_type);
extern struct Accelerator **ScnGetAcceleratorList(const struct Scene *scn);
extern struct Accelerator *ScnGetAccelerator(const struct Scene *scn, int index);
extern size_t ScnGetAcceleratorCount(const struct Scene *scn);

/* FrameBuffer */
extern struct FrameBuffer *ScnNewFrameBuffer(struct Scene *scn);
extern struct FrameBuffer **ScnGetFrameBufferList(const struct Scene *scn);
extern struct FrameBuffer *ScnGetFrameBuffer(const struct Scene *scn, int index);
extern size_t ScnGetFrameBufferCount(const struct Scene *scn);

/* ObjectGroup */
extern struct ObjectGroup *ScnNewObjectGroup(struct Scene *scn);
extern struct ObjectGroup **ScnGetObjectGroupList(const struct Scene *scn);
extern struct ObjectGroup *ScnGetObjectGroup(const struct Scene *scn, int index);
extern size_t ScnGetObjectGroupCount(const struct Scene *scn);

/* Renderer */
extern struct Renderer *ScnNewRenderer(struct Scene *scn);
extern struct Renderer **ScnGetRendererList(const struct Scene *scn);
extern struct Renderer *ScnGetRenderer(const struct Scene *scn, int index);
extern size_t ScnGetRendererCount(const struct Scene *scn);

/* Texture */
extern struct Texture *ScnNewTexture(struct Scene *scn);
extern struct Texture **ScnGetTextureList(const struct Scene *scn);
extern struct Texture *ScnGetTexture(const struct Scene *scn, int index);
extern size_t ScnGetTextureCount(const struct Scene *scn);

/* Camera */
extern struct Camera *ScnNewCamera(struct Scene *scn, const char *type);
extern struct Camera **ScnGetCameraList(const struct Scene *scn);
extern struct Camera *ScnGetCamera(const struct Scene *scn, int index);
extern size_t ScnGetCameraCount(const struct Scene *scn);

/* Plugin */
extern struct Plugin *ScnOpenPlugin(struct Scene *scn, const char *filename);
extern struct Plugin **ScnGetPluginList(const struct Scene *scn);
extern size_t ScnGetPluginCount(const struct Scene *scn);

/* Shader */
extern struct Shader *ScnNewShader(struct Scene *scn, const struct Plugin *plugin);
extern struct Shader **ScnGetShaderList(const struct Scene *scn);
extern struct Shader *ScnGetShader(const struct Scene *scn, int index);
extern size_t ScnGetShaderCount(const struct Scene *scn);

/* Volume */
extern struct Volume *ScnNewVolume(struct Scene *scn);
extern struct Volume **ScnGetVolumeList(const struct Scene *scn);
extern struct Volume *ScnGetVolume(const struct Scene *scn, int index);
extern size_t ScnGetVolumeCount(const struct Scene *scn);

/* Curve */
extern struct Curve *ScnNewCurve(struct Scene *scn);
extern struct Curve **ScnGetCurveList(const struct Scene *scn);
extern struct Curve *ScnGetCurve(const struct Scene *scn, int index);
extern size_t ScnGetCurveCount(const struct Scene *scn);

/* Light */
extern struct Light *ScnNewLight(struct Scene *scn, const char *type);
extern struct Light **ScnGetLightList(const struct Scene *scn);
extern struct Light *ScnGetLight(const struct Scene *scn, int index);
extern size_t ScnGetLightCount(const struct Scene *scn);

/* Mesh */
extern struct Mesh *ScnNewMesh(struct Scene *scn);
extern struct Mesh **ScnGetMeshList(const struct Scene *scn);
extern struct Mesh *ScnGetMesh(const struct Scene *scn, int index);
extern size_t ScnGetMeshCount(const struct Scene *scn);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

