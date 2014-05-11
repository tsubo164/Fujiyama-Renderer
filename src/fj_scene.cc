/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_scene.h"
#include "fj_memory.h"
#include "fj_array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST(scene,type) ((scene)->type ## List)

#define NEW_LIST(scene,type) do { \
  LIST(scene,type) = ArrNew(sizeof(struct type *)); \
  /* printf(#type"List newed\n"); */ \
} while(0)

#define FREE_LIST(scene,type,freefn) do { \
  int i; \
  struct type **nodes; \
  if (LIST(scene,type) == NULL) break; \
  nodes = (struct type ** ) LIST(scene,type)->data; \
  for (i = 0; i < (int) LIST(scene,type)->nelems; i++) { \
    freefn(nodes[i]); \
    /* printf("\t"#type" %d: freed\n", i); */ \
  } \
  ArrFree(LIST(scene,type)); \
  /* printf(#type"List freed\n"); */ \
} while(0)

#define DEFINE_LIST_FUNCTIONS(Type) \
size_t ScnGet##Type##Count(const struct Scene *scene) \
{ \
  return (scene)->Type##List->nelems; \
} \
struct Type **ScnGet##Type##List(const struct Scene *scene) \
{ \
  return (struct Type **) (scene)->Type##List->data; \
} \
struct Type *ScnGet##Type(const struct Scene *scene, int index) \
{ \
  if((index) < 0 || (index) >= (int) ScnGet##Type##Count((scene))) return NULL; \
  return ScnGet##Type##List((scene))[(index)]; \
}

namespace fj {

struct Scene {
  struct Array *ObjectInstanceList;
  struct Array *AcceleratorList;
  struct Array *FrameBufferList;
  struct Array *ObjectGroupList;
  struct Array *PointCloudList;
  struct Array *TurbulenceList;
  struct Array *ProcedureList;
  struct Array *RendererList;
  struct Array *TextureList;
  struct Array *CameraList;
  struct Array *ConfigList;
  struct Array *PluginList;
  struct Array *ShaderList;
  struct Array *VolumeList;
  struct Array *CurveList;
  struct Array *LightList;
  struct Array *MeshList;
};

static void new_all_node_list(struct Scene *scene);
static void free_all_node_list(struct Scene *scene);
static void *push_entry(struct Array *array, void *entry);

/* Scene */
struct Scene *ScnNew(void)
{
  struct Scene *scene = FJ_MEM_ALLOC(struct Scene);

  if (scene == NULL)
    return NULL;

  new_all_node_list(scene);
  return scene;
}

void ScnFree(struct Scene *scene)
{
  if (scene == NULL)
    return;

  free_all_node_list(scene);
  FJ_MEM_FREE(scene);
}

/* ObjectInstance */
struct ObjectInstance *ScnNewObjectInstance(struct Scene *scene)
{
  return (struct ObjectInstance *) push_entry(scene->ObjectInstanceList, ObjNew());
}

/* Accelerator */
struct Accelerator *ScnNewAccelerator(struct Scene *scene, int accelerator_type)
{
  return (struct Accelerator *) push_entry(scene->AcceleratorList, AccNew(accelerator_type));
}

/* FrameBuffer */
struct FrameBuffer *ScnNewFrameBuffer(struct Scene *scene)
{
  return (struct FrameBuffer *) push_entry(scene->FrameBufferList, FbNew());
}

/* ObjectGroup */
struct ObjectGroup *ScnNewObjectGroup(struct Scene *scene)
{
  return (struct ObjectGroup *) push_entry(scene->ObjectGroupList, ObjGroupNew());
}

/* PointCloud */
struct PointCloud *ScnNewPointCloud(struct Scene *scene)
{
  return (struct PointCloud *) push_entry(scene->PointCloudList, PtcNew());
}

/* Turbulence */
struct Turbulence *ScnNewTurbulence(struct Scene *scene)
{
  return (struct Turbulence *) push_entry(scene->TurbulenceList, TrbNew());
}

/* Procedure */
struct Procedure *ScnNewProcedure(struct Scene *scene, const struct Plugin *plugin)
{
  return (struct Procedure *) push_entry(scene->ProcedureList, PrcNew(plugin));
}

/* Renderer */
struct Renderer *ScnNewRenderer(struct Scene *scene)
{
  return (struct Renderer *) push_entry(scene->RendererList, RdrNew());
}

/* Texture */
struct Texture *ScnNewTexture(struct Scene *scene)
{
  return (struct Texture *) push_entry(scene->TextureList, TexNew());
}

/* Camera */
struct Camera *ScnNewCamera(struct Scene *scene, const char *type)
{
  return (struct Camera *) push_entry(scene->CameraList, CamNew(type));
}

/* Plugin */
struct Plugin *ScnOpenPlugin(struct Scene *scene, const char *filename)
{
  return (struct Plugin *) push_entry(scene->PluginList, PlgOpen(filename));
}

/* Shader */
struct Shader *ScnNewShader(struct Scene *scene, const struct Plugin *plugin)
{
  return (struct Shader *) push_entry(scene->ShaderList, ShdNew(plugin));
}

/* Volume */
struct Volume *ScnNewVolume(struct Scene *scene)
{
  return (struct Volume *) push_entry(scene->VolumeList, VolNew());
}

/* Curve */
struct Curve *ScnNewCurve(struct Scene *scene)
{
  return (struct Curve *) push_entry(scene->CurveList, CrvNew());
}

/* Light */
struct Light *ScnNewLight(struct Scene *scene, int light_type)
{
  return (struct Light *) push_entry(scene->LightList, LgtNew(light_type));
}

/* Mesh */
struct Mesh *ScnNewMesh(struct Scene *scene)
{
  return (struct Mesh *) push_entry(scene->MeshList, MshNew());
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

static void new_all_node_list(struct Scene *scene)
{
  NEW_LIST(scene, ObjectInstance);
  NEW_LIST(scene, Accelerator);
  NEW_LIST(scene, FrameBuffer);
  NEW_LIST(scene, ObjectGroup);
  NEW_LIST(scene, PointCloud);
  NEW_LIST(scene, Turbulence);
  NEW_LIST(scene, Procedure);
  NEW_LIST(scene, Renderer);
  NEW_LIST(scene, Texture);
  NEW_LIST(scene, Camera);
  NEW_LIST(scene, Plugin);
  NEW_LIST(scene, Shader);
  NEW_LIST(scene, Volume);
  NEW_LIST(scene, Curve);
  NEW_LIST(scene, Light);
  NEW_LIST(scene, Mesh);
}

static void free_all_node_list(struct Scene *scene)
{
  FREE_LIST(scene, ObjectInstance, ObjFree);
  FREE_LIST(scene, Accelerator, AccFree);
  FREE_LIST(scene, FrameBuffer, FbFree);
  FREE_LIST(scene, ObjectGroup, ObjGroupFree);
  FREE_LIST(scene, PointCloud, PtcFree);
  FREE_LIST(scene, Turbulence, TrbFree);
  FREE_LIST(scene, Procedure, PrcFree);
  FREE_LIST(scene, Renderer, RdrFree);
  FREE_LIST(scene, Texture, TexFree);
  FREE_LIST(scene, Camera, CamFree);
  FREE_LIST(scene, Shader, ShdFree);
  FREE_LIST(scene, Volume, VolFree);
  FREE_LIST(scene, Curve, CrvFree);
  FREE_LIST(scene, Light, LgtFree);
  FREE_LIST(scene, Mesh, MshFree);

  /* plugins should be freed the last since they contain freeing functions for others */
  FREE_LIST(scene, Plugin, PlgClose);
}

static void *push_entry(struct Array *array, void *entry)
{
  if (entry == NULL)
    return NULL;

  ArrPushPointer(array, entry);
  return entry;
}

} // namespace xxx
