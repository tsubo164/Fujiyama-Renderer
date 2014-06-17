// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SCENE_H
#define FJ_SCENE_H

#include "fj_object_instance.h"
#include "fj_object_group.h"
#include "fj_accelerator.h"
#include "fj_framebuffer.h"
#include "fj_point_cloud.h"
#include "fj_turbulence.h"
#include "fj_procedure.h"
#include "fj_renderer.h"
#include "fj_texture.h"
#include "fj_camera.h"
#include "fj_plugin.h"
#include "fj_shader.h"
#include "fj_volume.h"
#include "fj_curve.h"
#include "fj_light.h"
#include "fj_mesh.h"

namespace fj {

// TODO TMP
enum AcceleratorType {
  ACC_GRID = 0,
  ACC_BVH
};

class Scene;

// Scene
extern Scene *ScnNew(void);
extern void ScnFree(Scene *scene);

// ObjectInstance
extern ObjectInstance *ScnNewObjectInstance(Scene *scene);
extern ObjectInstance **ScnGetObjectInstanceList(const Scene *scene);
extern ObjectInstance *ScnGetObjectInstance(const Scene *scene, int index);
extern size_t ScnGetObjectInstanceCount(const Scene *scene);

// Accelerator
extern Accelerator *ScnNewAccelerator(Scene *scene, int accelerator_type);
extern Accelerator **ScnGetAcceleratorList(const Scene *scene);
extern Accelerator *ScnGetAccelerator(const Scene *scene, int index);
extern size_t ScnGetAcceleratorCount(const Scene *scene);

// FrameBuffer
extern FrameBuffer *ScnNewFrameBuffer(Scene *scene);
extern FrameBuffer **ScnGetFrameBufferList(const Scene *scene);
extern FrameBuffer *ScnGetFrameBuffer(const Scene *scene, int index);
extern size_t ScnGetFrameBufferCount(const Scene *scene);

// ObjectGroup
extern ObjectGroup *ScnNewObjectGroup(Scene *scene);
extern ObjectGroup **ScnGetObjectGroupList(const Scene *scene);
extern ObjectGroup *ScnGetObjectGroup(const Scene *scene, int index);
extern size_t ScnGetObjectGroupCount(const Scene *scene);

// PointCloud
extern PointCloud *ScnNewPointCloud(Scene *scene);
extern PointCloud **ScnGetPointCloudList(const Scene *scene);
extern PointCloud *ScnGetPointCloud(const Scene *scene, int index);
extern size_t ScnGetPointCloudCount(const Scene *scene);

// Turbulence
extern Turbulence *ScnNewTurbulence(Scene *scene);
extern Turbulence **ScnGetTurbulenceList(const Scene *scene);
extern Turbulence *ScnGetTurbulence(const Scene *scene, int index);
extern size_t ScnGetTurbulenceCount(const Scene *scene);

// Procedure
extern Procedure *ScnNewProcedure(Scene *scene, const Plugin *plugin);
extern Procedure **ScnGetProcedureList(const Scene *scene);
extern Procedure *ScnGetProcedure(const Scene *scene, int index);
extern size_t ScnGetProcedureCount(const Scene *scene);

// Renderer
extern Renderer *ScnNewRenderer(Scene *scene);
extern Renderer **ScnGetRendererList(const Scene *scene);
extern Renderer *ScnGetRenderer(const Scene *scene, int index);
extern size_t ScnGetRendererCount(const Scene *scene);

// Texture
extern Texture *ScnNewTexture(Scene *scene);
extern Texture **ScnGetTextureList(const Scene *scene);
extern Texture *ScnGetTexture(const Scene *scene, int index);
extern size_t ScnGetTextureCount(const Scene *scene);

// Camera
extern Camera *ScnNewCamera(Scene *scene, const char *type);
extern Camera **ScnGetCameraList(const Scene *scene);
extern Camera *ScnGetCamera(const Scene *scene, int index);
extern size_t ScnGetCameraCount(const Scene *scene);

// Plugin
extern Plugin *ScnOpenPlugin(Scene *scene, const char *filename);
extern Plugin **ScnGetPluginList(const Scene *scene);
extern size_t ScnGetPluginCount(const Scene *scene);

// Shader
extern Shader *ScnNewShader(Scene *scene, const Plugin *plugin);
extern Shader **ScnGetShaderList(const Scene *scene);
extern Shader *ScnGetShader(const Scene *scene, int index);
extern size_t ScnGetShaderCount(const Scene *scene);

// Volume
extern Volume *ScnNewVolume(Scene *scene);
extern Volume **ScnGetVolumeList(const Scene *scene);
extern Volume *ScnGetVolume(const Scene *scene, int index);
extern size_t ScnGetVolumeCount(const Scene *scene);

// Curve
extern Curve *ScnNewCurve(Scene *scene);
extern Curve **ScnGetCurveList(const Scene *scene);
extern Curve *ScnGetCurve(const Scene *scene, int index);
extern size_t ScnGetCurveCount(const Scene *scene);

// Light
extern Light *ScnNewLight(Scene *scene, int light_type);
extern Light **ScnGetLightList(const Scene *scene);
extern Light *ScnGetLight(const Scene *scene, int index);
extern size_t ScnGetLightCount(const Scene *scene);

// Mesh
extern Mesh *ScnNewMesh(Scene *scene);
extern Mesh **ScnGetMeshList(const Scene *scene);
extern Mesh *ScnGetMesh(const Scene *scene, int index);
extern size_t ScnGetMeshCount(const Scene *scene);

} // namespace xxx

#endif // FJ_XXX_H
