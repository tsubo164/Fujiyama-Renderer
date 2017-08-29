// Copyright (c) 2011-2017 Hiroshi Tsubokawa
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
#include <vector>

namespace fj {

class Scene {
public:
  Scene();
  ~Scene();

  // ObjectInstance
  ObjectInstance *NewObjectInstance();
  ObjectInstance **GetObjectInstanceList() const;
  ObjectInstance *GetObjectInstance(int index) const;
  size_t GetObjectInstanceCount() const;

  // Accelerator
  Accelerator *NewGridAccelerator();
  Accelerator *NewBVHAccelerator();
  Accelerator **GetAcceleratorList() const;
  Accelerator *GetAccelerator(int index) const;
  size_t GetAcceleratorCount() const;

  // FrameBuffer
  FrameBuffer *NewFrameBuffer();
  FrameBuffer **GetFrameBufferList() const;
  FrameBuffer *GetFrameBuffer(int index) const;
  size_t GetFrameBufferCount() const;

  // ObjectGroup
  ObjectGroup *NewObjectGroup();
  ObjectGroup **GetObjectGroupList() const;
  ObjectGroup *GetObjectGroup(int index) const;
  size_t GetObjectGroupCount() const;

  // PointCloud
  PointCloud *NewPointCloud();
  PointCloud **GetPointCloudList() const;
  PointCloud *GetPointCloud(int index) const;
  size_t GetPointCloudCount() const;

  // Turbulence
  Turbulence *NewTurbulence();
  Turbulence **GetTurbulenceList() const;
  Turbulence *GetTurbulence(int index) const;
  size_t GetTurbulenceCount() const;

  // Procedure
  Procedure *NewProcedure(Plugin *plugin);
  Procedure **GetProcedureList() const;
  Procedure *GetProcedure(int index) const;
  size_t GetProcedureCount() const;

  // Renderer
  Renderer *NewRenderer();
  Renderer **GetRendererList() const;
  Renderer *GetRenderer(int index) const;
  size_t GetRendererCount() const;

  // Texture
  Texture *NewTexture();
  Texture **GetTextureList() const;
  Texture *GetTexture(int index) const;
  size_t GetTextureCount() const;

  // Camera
  Camera *NewCamera(const char *type);
  Camera **GetCameraList() const;
  Camera *GetCamera(int index) const;
  size_t GetCameraCount() const;

  // Plugin
  Plugin *OpenPlugin(const char *filename);
  Plugin **GetPluginList() const;
  Plugin *GetPlugin(int index) const;
  size_t GetPluginCount() const;

  // Shader
  Shader *NewShader(Plugin *plugin);
  Shader **GetShaderList() const;
  Shader *GetShader(int index) const;
  size_t GetShaderCount() const;

  // Volume
  Volume *NewVolume();
  Volume **GetVolumeList() const;
  Volume *GetVolume(int index) const;
  size_t GetVolumeCount() const;

  // Curve
  Curve *NewCurve();
  Curve **GetCurveList() const;
  Curve *GetCurve(int index) const;
  size_t GetCurveCount() const;

  // Light
  Light *NewRectangleLight();
  Light *NewSphereLight();
  Light *NewPointLight();
  Light *NewDomeLight();
  Light **GetLightList() const;
  Light *GetLight(int index) const;
  size_t GetLightCount() const;

  // Mesh
  Mesh *NewMesh();
  Mesh **GetMeshList() const;
  Mesh *GetMesh(int index) const;
  size_t GetMeshCount() const;

private:
  void free_all_node_list();

  std::vector<ObjectInstance *> ObjectInstanceList;
  std::vector<Accelerator *> AcceleratorList;
  std::vector<FrameBuffer *> FrameBufferList;
  std::vector<ObjectGroup *> ObjectGroupList;
  std::vector<PointCloud *> PointCloudList;
  std::vector<Turbulence *> TurbulenceList;
  std::vector<Procedure *> ProcedureList;
  std::vector<Renderer *> RendererList;
  std::vector<Texture *> TextureList;
  std::vector<Camera *> CameraList;
  std::vector<Plugin *> PluginList;
  std::vector<Shader *> ShaderList;
  std::vector<Volume *> VolumeList;
  std::vector<Curve *> CurveList;
  std::vector<Light *> LightList;
  std::vector<Mesh *> MeshList;
};

} // namespace xxx

#endif // FJ_XXX_H
