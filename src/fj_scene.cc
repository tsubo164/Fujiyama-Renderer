// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_scene.h"
#include "fj_grid_accelerator.h"
#include "fj_bvh_accelerator.h"
#include <cassert>

#define DEFINE_LIST_FUNCTIONS(Type) \
size_t Scene::Get##Type##Count() const \
{ \
  return Type##List.size(); \
} \
Type **Scene::Get##Type##List() const \
{ \
  return (Type **) &Type##List[0]; \
} \
Type *Scene::Get##Type(int index) const \
{ \
  if((index) < 0 || (index) >= (int) Get##Type##Count()) return NULL; \
  return Get##Type##List()[(index)]; \
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

Scene::Scene()
{
}

Scene::~Scene()
{
  free_all_node_list();
}

// ObjectInstance
ObjectInstance *Scene::NewObjectInstance()
{
  ObjectInstance *object = new ObjectInstance();
  return push_entry_(ObjectInstanceList, object);
}

// Accelerator
Accelerator *Scene::NewAccelerator(int accelerator_type)
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
  return push_entry_(AcceleratorList, acc);
}

// FrameBuffer
FrameBuffer *Scene::NewFrameBuffer()
{
  FrameBuffer *framebuffer = new FrameBuffer();
  return push_entry_(FrameBufferList, framebuffer);
}

// ObjectGroup
ObjectGroup *Scene::NewObjectGroup()
{
  ObjectGroup *group = new ObjectGroup();
  return push_entry_(ObjectGroupList, group);
}

// PointCloud
PointCloud *Scene::NewPointCloud()
{
  PointCloud *ptc = new PointCloud();
  return push_entry_(PointCloudList, ptc);
}

// Turbulence
Turbulence *Scene::NewTurbulence()
{
  Turbulence *turbulence = new Turbulence();
  return push_entry_(TurbulenceList, turbulence);
}

// Procedure
Procedure *Scene::NewProcedure(const Plugin *plugin)
{
  Procedure *procedure = new Procedure();
  procedure->Initialize(plugin);
  return push_entry_(ProcedureList, procedure);
}

// Renderer
Renderer *Scene::NewRenderer()
{
  Renderer *renderer = new Renderer();
  return push_entry_(RendererList, renderer);
}

// Texture
Texture *Scene::NewTexture()
{
  Texture *texture = new Texture();
  return push_entry_(TextureList, texture);
}

// Camera
Camera *Scene::NewCamera(const char *type)
{
  Camera *camera = new Camera();
  return push_entry_(CameraList, camera);
}

// Plugin
Plugin *Scene::OpenPlugin(const char *filename)
{
  Plugin *plugin = new Plugin();
  plugin->Open(filename);
  return push_entry_(PluginList, plugin);
}

// Shader
Shader *Scene::NewShader(const Plugin *plugin)
{
  Shader *shader = new Shader();
  shader->Initialize(plugin);
  return push_entry_(ShaderList, shader);
}

// Volume
Volume *Scene::NewVolume()
{
  Volume *volume = new Volume();
  return push_entry_(VolumeList, volume);
}

// Curve
Curve *Scene::NewCurve()
{
  Curve *curve = new Curve();
  return push_entry_(CurveList, curve);
}

// Light
Light *Scene::NewLight(int light_type)
{
  Light *light = new Light();
  light->SetLightType(light_type);
  return push_entry_(LightList, light);
}

// Mesh
Mesh *Scene::NewMesh()
{
  Mesh *mesh = new Mesh();
  return push_entry_(MeshList, mesh);
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
