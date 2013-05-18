/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef SCENE_H
#define SCENE_H

#include "ObjectInstance.h"
#include "Accelerator.h"
#include "FrameBuffer.h"
#include "ObjectGroup.h"
#include "PointCloud.h"
#include "Turbulence.h"
#include "Procedure.h"
#include "Renderer.h"
#include "Texture.h"
#include "Camera.h"
#include "Plugin.h"
#include "Shader.h"
#include "Volume.h"
#include "Curve.h"
#include "Light.h"
#include "Mesh.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Scene;

/* Scene */
extern struct Scene *ScnNew(void);
extern void ScnFree(struct Scene *scene);

/* ObjectInstance */
extern struct ObjectInstance *ScnNewObjectInstance(struct Scene *scene);
extern struct ObjectInstance **ScnGetObjectInstanceList(const struct Scene *scene);
extern struct ObjectInstance *ScnGetObjectInstance(const struct Scene *scene, int index);
extern size_t ScnGetObjectInstanceCount(const struct Scene *scene);

/* Accelerator */
extern struct Accelerator *ScnNewAccelerator(struct Scene *scene, int accelerator_type);
extern struct Accelerator **ScnGetAcceleratorList(const struct Scene *scene);
extern struct Accelerator *ScnGetAccelerator(const struct Scene *scene, int index);
extern size_t ScnGetAcceleratorCount(const struct Scene *scene);

/* FrameBuffer */
extern struct FrameBuffer *ScnNewFrameBuffer(struct Scene *scene);
extern struct FrameBuffer **ScnGetFrameBufferList(const struct Scene *scene);
extern struct FrameBuffer *ScnGetFrameBuffer(const struct Scene *scene, int index);
extern size_t ScnGetFrameBufferCount(const struct Scene *scene);

/* ObjectGroup */
extern struct ObjectGroup *ScnNewObjectGroup(struct Scene *scene);
extern struct ObjectGroup **ScnGetObjectGroupList(const struct Scene *scene);
extern struct ObjectGroup *ScnGetObjectGroup(const struct Scene *scene, int index);
extern size_t ScnGetObjectGroupCount(const struct Scene *scene);

/* PointCloud */
extern struct PointCloud *ScnNewPointCloud(struct Scene *scene);
extern struct PointCloud **ScnGetPointCloudList(const struct Scene *scene);
extern struct PointCloud *ScnGetPointCloud(const struct Scene *scene, int index);
extern size_t ScnGetPointCloudCount(const struct Scene *scene);

/* Turbulence */
extern struct Turbulence *ScnNewTurbulence(struct Scene *scene);
extern struct Turbulence **ScnGetTurbulenceList(const struct Scene *scene);
extern struct Turbulence *ScnGetTurbulence(const struct Scene *scene, int index);
extern size_t ScnGetTurbulenceCount(const struct Scene *scene);

/* Procedure */
extern struct Procedure *ScnNewProcedure(struct Scene *scene, const struct Plugin *plugin);
extern struct Procedure **ScnGetProcedureList(const struct Scene *scene);
extern struct Procedure *ScnGetProcedure(const struct Scene *scene, int index);
extern size_t ScnGetProcedureCount(const struct Scene *scene);

/* Renderer */
extern struct Renderer *ScnNewRenderer(struct Scene *scene);
extern struct Renderer **ScnGetRendererList(const struct Scene *scene);
extern struct Renderer *ScnGetRenderer(const struct Scene *scene, int index);
extern size_t ScnGetRendererCount(const struct Scene *scene);

/* Texture */
extern struct Texture *ScnNewTexture(struct Scene *scene);
extern struct Texture **ScnGetTextureList(const struct Scene *scene);
extern struct Texture *ScnGetTexture(const struct Scene *scene, int index);
extern size_t ScnGetTextureCount(const struct Scene *scene);

/* Camera */
extern struct Camera *ScnNewCamera(struct Scene *scene, const char *type);
extern struct Camera **ScnGetCameraList(const struct Scene *scene);
extern struct Camera *ScnGetCamera(const struct Scene *scene, int index);
extern size_t ScnGetCameraCount(const struct Scene *scene);

/* Plugin */
extern struct Plugin *ScnOpenPlugin(struct Scene *scene, const char *filename);
extern struct Plugin **ScnGetPluginList(const struct Scene *scene);
extern size_t ScnGetPluginCount(const struct Scene *scene);

/* Shader */
extern struct Shader *ScnNewShader(struct Scene *scene, const struct Plugin *plugin);
extern struct Shader **ScnGetShaderList(const struct Scene *scene);
extern struct Shader *ScnGetShader(const struct Scene *scene, int index);
extern size_t ScnGetShaderCount(const struct Scene *scene);

/* Volume */
extern struct Volume *ScnNewVolume(struct Scene *scene);
extern struct Volume **ScnGetVolumeList(const struct Scene *scene);
extern struct Volume *ScnGetVolume(const struct Scene *scene, int index);
extern size_t ScnGetVolumeCount(const struct Scene *scene);

/* Curve */
extern struct Curve *ScnNewCurve(struct Scene *scene);
extern struct Curve **ScnGetCurveList(const struct Scene *scene);
extern struct Curve *ScnGetCurve(const struct Scene *scene, int index);
extern size_t ScnGetCurveCount(const struct Scene *scene);

/* Light */
extern struct Light *ScnNewLight(struct Scene *scene, int light_type);
extern struct Light **ScnGetLightList(const struct Scene *scene);
extern struct Light *ScnGetLight(const struct Scene *scene, int index);
extern size_t ScnGetLightCount(const struct Scene *scene);

/* Mesh */
extern struct Mesh *ScnNewMesh(struct Scene *scene);
extern struct Mesh **ScnGetMeshList(const struct Scene *scene);
extern struct Mesh *ScnGetMesh(const struct Scene *scene, int index);
extern size_t ScnGetMeshCount(const struct Scene *scene);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

