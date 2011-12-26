/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Scene.h"
#include "Array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST(scn,type) ((scn)->type ## List)

#define NEW_LIST(scn,type) do { \
	LIST(scn,type) = ArrNew(sizeof(struct type *)); \
	/* printf(#type"List newed\n"); */ \
} while(0)

#define FREE_LIST(scn,type,freefn) do { \
	int i; \
	struct type **nodes; \
	if (LIST(scn,type) == NULL) break; \
	nodes = (struct type ** ) LIST(scn,type)->data; \
	for (i = 0; i < LIST(scn,type)->nelems; i++) { \
		freefn(nodes[i]); \
		/* printf("\t"#type" %d: freed\n", i); */ \
	} \
	ArrFree(LIST(scn,type)); \
	/* printf(#type"List freed\n"); */ \
} while(0)

#define DEFINE_LIST_FUNCTIONS(Type) \
size_t ScnGet##Type##Count(const struct Scene *scn) \
{ \
	return (scn)->Type##List->nelems; \
} \
struct Type **ScnGet##Type##List(const struct Scene *scn) \
{ \
	return (struct Type **) (scn)->Type ## List->data; \
} \
struct Type *ScnGet##Type(const struct Scene *scn, int index) \
{ \
	if((index) < 0 || (index) >= ScnGet##Type##Count((scn))) return NULL; \
	return ScnGet##Type##List((scn))[(index)]; \
}

struct Scene {
	struct Array *ObjectInstanceList;
	struct Array *AcceleratorList;
	struct Array *FrameBufferList;
	struct Array *ObjectGroupList;
	struct Array *RendererList;
	struct Array *TextureList;
	struct Array *CameraList;
	struct Array *ConfigList;
	struct Array *PluginList;
	struct Array *ShaderList;
	struct Array *CurveList;
	struct Array *LightList;
	struct Array *MeshList;
};

static void new_all_node_list(struct Scene *scn);
static void free_all_node_list(struct Scene *scn);
static void *push_entry(struct Array *array, void *entry);

/* Scene */
struct Scene *ScnNew(void)
{
	struct Scene *scn;

	scn = (struct Scene *) malloc(sizeof(struct Scene));
	if (scn == NULL)
		return NULL;

	new_all_node_list(scn);
	return scn;
}

void ScnFree(struct Scene *scn)
{
	if (scn == NULL)
		return;

	free_all_node_list(scn);
	free(scn);
}

/* ObjectInstance */
struct ObjectInstance *ScnNewObjectInstance(struct Scene *scn, const struct Accelerator *acc)
{
	return push_entry(scn->ObjectInstanceList, ObjNew(acc));
}

/* Accelerator */
struct Accelerator *ScnNewAccelerator(struct Scene *scn, int accelerator_type)
{
	return push_entry(scn->AcceleratorList, AccNew(accelerator_type));
}

/* FrameBuffer */
struct FrameBuffer *ScnNewFrameBuffer(struct Scene *scn)
{
	return push_entry(scn->FrameBufferList, FbNew());
}

/* ObjectGroup */
struct ObjectGroup *ScnNewObjectGroup(struct Scene *scn)
{
	return push_entry(scn->ObjectGroupList, ObjGroupNew());
}

/* Renderer */
struct Renderer *ScnNewRenderer(struct Scene *scn)
{
	return push_entry(scn->RendererList, RdrNew());
}

/* Texture */
struct Texture *ScnNewTexture(struct Scene *scn)
{
	return push_entry(scn->TextureList, TexNew());
}

/* Camera */
struct Camera *ScnNewCamera(struct Scene *scn, const char *type)
{
	return push_entry(scn->CameraList, CamNew(type));
}

/* Plugin */
struct Plugin *ScnOpenPlugin(struct Scene *scn, const char *filename)
{
	return push_entry(scn->PluginList, PlgOpen(filename));
}

/* Shader */
struct Shader *ScnNewShader(struct Scene *scn, const struct Plugin *plugin)
{
	return push_entry(scn->ShaderList, ShdNew(plugin));
}

/* Curve */
struct Curve *ScnNewCurve(struct Scene *scn)
{
	return push_entry(scn->CurveList, CrvNew());
}

/* Light */
struct Light *ScnNewLight(struct Scene *scn, const char *type)
{
	return push_entry(scn->LightList, LgtNew(type));
}

/* Mesh */
struct Mesh *ScnNewMesh(struct Scene *scn)
{
	return push_entry(scn->MeshList, MshNew());
}

DEFINE_LIST_FUNCTIONS(ObjectInstance)
DEFINE_LIST_FUNCTIONS(Accelerator)
DEFINE_LIST_FUNCTIONS(FrameBuffer)
DEFINE_LIST_FUNCTIONS(ObjectGroup)
DEFINE_LIST_FUNCTIONS(Renderer)
DEFINE_LIST_FUNCTIONS(Texture)
DEFINE_LIST_FUNCTIONS(Camera)
DEFINE_LIST_FUNCTIONS(Plugin)
DEFINE_LIST_FUNCTIONS(Shader)
DEFINE_LIST_FUNCTIONS(Curve)
DEFINE_LIST_FUNCTIONS(Light)
DEFINE_LIST_FUNCTIONS(Mesh)

static void new_all_node_list(struct Scene *scn)
{
	NEW_LIST(scn, ObjectInstance);
	NEW_LIST(scn, Accelerator);
	NEW_LIST(scn, FrameBuffer);
	NEW_LIST(scn, ObjectGroup);
	NEW_LIST(scn, Renderer);
	NEW_LIST(scn, Texture);
	NEW_LIST(scn, Camera);
	NEW_LIST(scn, Plugin);
	NEW_LIST(scn, Shader);
	NEW_LIST(scn, Curve);
	NEW_LIST(scn, Light);
	NEW_LIST(scn, Mesh);
}

static void free_all_node_list(struct Scene *scn)
{
	FREE_LIST(scn, ObjectInstance, ObjFree);
	FREE_LIST(scn, Accelerator, AccFree);
	FREE_LIST(scn, FrameBuffer, FbFree);
	FREE_LIST(scn, ObjectGroup, ObjGroupFree);
	FREE_LIST(scn, Texture, TexFree);
	FREE_LIST(scn, Camera, CamFree);
	FREE_LIST(scn, Shader, ShdFree);
	FREE_LIST(scn, Curve, CrvFree);
	FREE_LIST(scn, Light, LgtFree);
	FREE_LIST(scn, Mesh, MshFree);

	/* plugins should be freed the last since they contain free functions for others */
	FREE_LIST(scn, Plugin, PlgClose);
}

static void *push_entry(struct Array *array, void *entry)
{
	if (entry == NULL)
		return NULL;

	ArrPushPointer(array, entry);
	return entry;
}

