// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_scene.h"
#include "fj_grid_accelerator.h"
#include "fj_bvh_accelerator.h"

#include <vector>
#include <cassert>

#define DEFINE_LIST_FUNCTIONS(Type) \
size_t ScnGet##Type##Count(const Scene *scene) \
{ \
  return (scene)->Type##List.size(); \
} \
Type **ScnGet##Type##List(const Scene *scene) \
{ \
  return (Type **) &(scene)->Type##List[0]; \
} \
Type *ScnGet##Type(const Scene *scene, int index) \
{ \
  if((index) < 0 || (index) >= (int) ScnGet##Type##Count((scene))) return NULL; \
  return ScnGet##Type##List((scene))[(index)]; \
}

namespace fj {

template<typename T>
static inline
void delete_entries(std::vector<T *> &entry_list)
{
  for (size_t i = 0; i < entry_list.size(); i++) {
    delete entry_list[i];
  }
}

template<typename T>
static inline
T *push_entry_(std::vector<T *> &entry_list, T *entry)
{
  entry_list.push_back(entry);
  return entry;
}

class Scene {
public:
  Scene();
  ~Scene();

public:
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

Scene::Scene()
{
}

Scene::~Scene()
{
  free_all_node_list();
}

// Scene
Scene *ScnNew(void)
{
  return new Scene();
}

void ScnFree(Scene *scene)
{
  delete scene;
}

// ObjectInstance
ObjectInstance *ScnNewObjectInstance(Scene *scene)
{
  ObjectInstance *object = new ObjectInstance();
  return push_entry_(scene->ObjectInstanceList, object);
}

// Accelerator
Accelerator *ScnNewAccelerator(Scene *scene, int accelerator_type)
{
  Accelerator *acc = NULL;
  switch (accelerator_type) {
  case ACC_GRID:
    acc = new GridAccelerator();
    break;
  case ACC_BVH:
    acc = new BVHAccelerator();
    break;
  default:
    assert(!"invalid accelerator type");
    break;
  }
  return push_entry_(scene->AcceleratorList, acc);
}

// FrameBuffer
FrameBuffer *ScnNewFrameBuffer(Scene *scene)
{
  FrameBuffer *framebuffer = new FrameBuffer();
  return push_entry_(scene->FrameBufferList, framebuffer);
}

// ObjectGroup
ObjectGroup *ScnNewObjectGroup(Scene *scene)
{
  ObjectGroup *group = new ObjectGroup();
  return push_entry_(scene->ObjectGroupList, group);
}

// PointCloud
PointCloud *ScnNewPointCloud(Scene *scene)
{
  PointCloud *ptc = new PointCloud();
  return push_entry_(scene->PointCloudList, ptc);
}

// Turbulence
Turbulence *ScnNewTurbulence(Scene *scene)
{
  Turbulence *turbulence = new Turbulence();
  return push_entry_(scene->TurbulenceList, turbulence);
}

// Procedure
Procedure *ScnNewProcedure(Scene *scene, const Plugin *plugin)
{
  Procedure *procedure = new Procedure();
  procedure->Initialize(plugin);
  return push_entry_(scene->ProcedureList, procedure);
}

// Renderer
Renderer *ScnNewRenderer(Scene *scene)
{
  Renderer *renderer = new Renderer();
  return push_entry_(scene->RendererList, renderer);
}

// Texture
Texture *ScnNewTexture(Scene *scene)
{
  Texture *texture = new Texture();
  return push_entry_(scene->TextureList, texture);
}

// Camera
Camera *ScnNewCamera(Scene *scene, const char *type)
{
  Camera *camera = new Camera();
  return push_entry_(scene->CameraList, camera);
}

// Plugin
Plugin *ScnOpenPlugin(Scene *scene, const char *filename)
{
  Plugin *plugin = new Plugin();
  plugin->Open(filename);
  return push_entry_(scene->PluginList, plugin);
}

// Shader
Shader *ScnNewShader(Scene *scene, const Plugin *plugin)
{
  Shader *shader = new Shader();
  shader->Initialize(plugin);
  return push_entry_(scene->ShaderList, shader);
}

// Volume
Volume *ScnNewVolume(Scene *scene)
{
  Volume *volume = new Volume();
  return push_entry_(scene->VolumeList, volume);
}

// Curve
Curve *ScnNewCurve(Scene *scene)
{
  Curve *curve = new Curve();
  return push_entry_(scene->CurveList, curve);
}

// Light
Light *ScnNewLight(Scene *scene, int light_type)
{
  Light *light = new Light();
  light->SetLightType(light_type);
  return push_entry_(scene->LightList, light);
}

// Mesh
Mesh *ScnNewMesh(Scene *scene)
{
  Mesh *mesh = new Mesh();
  return push_entry_(scene->MeshList, mesh);
}

DEFINE_LIST_FUNCTIONS(ObjectInstance)
DEFINE_LIST_FUNCTIONS(Accelerator)
DEFINE_LIST_FUNCTIONS(FrameBuffer)
DEFINE_LIST_FUNCTIONS(ObjectGroup)
DEFINE_LIST_FUNCTIONS(PointCloud)
DEFINE_LIST_FUNCTIONS(Turbulence)
DEFINE_LIST_FUNCTIONS(Procedure)
DEFINE_LIST_FUNCTIONS(Renderer)
DEFINE_LIST_FUNCTIONS(Texture)
DEFINE_LIST_FUNCTIONS(Camera)
DEFINE_LIST_FUNCTIONS(Plugin)
DEFINE_LIST_FUNCTIONS(Shader)
DEFINE_LIST_FUNCTIONS(Volume)
DEFINE_LIST_FUNCTIONS(Curve)
DEFINE_LIST_FUNCTIONS(Light)
DEFINE_LIST_FUNCTIONS(Mesh)

void Scene::free_all_node_list()
{
  delete_entries(ObjectInstanceList);
  delete_entries(AcceleratorList);
  delete_entries(FrameBufferList);
  delete_entries(ObjectGroupList);
  delete_entries(PointCloudList);
  delete_entries(TurbulenceList);
  delete_entries(ProcedureList);
  delete_entries(RendererList);
  delete_entries(TextureList);
  delete_entries(CameraList);
  delete_entries(ShaderList);
  delete_entries(VolumeList);
  delete_entries(CurveList);
  delete_entries(LightList);
  delete_entries(MeshList);

  // plugins should be freed the last since they contain freeing functions for others
  delete_entries(PluginList);
}

} // namespace xxx
