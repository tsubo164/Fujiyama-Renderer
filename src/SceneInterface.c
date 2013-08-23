/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "SceneInterface.h"
#include "FrameBufferIO.h"
#include "PointCloudIO.h"
#include "PrimitiveSet.h"
#include "CurveIO.h"
#include "MeshIO.h"
#include "Shader.h"
#include "Scene.h"
#include "Box.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define GET_LAST_ADDED_ID(Type) (ScnGet##Type##Count(get_scene()) - 1)

enum { TYPE_ID_OFFSET = 10000000 };

enum EntryType {
  Type_Begin = 0,
  Type_ObjectInstance = 1,
  Type_Accelerator,
  Type_FrameBuffer,
  Type_ObjectGroup,
  Type_PointCloud,
  Type_Turbulence,
  Type_Procedure,
  Type_Renderer,
  Type_Texture,
  Type_Camera,
  Type_Plugin,
  Type_Shader,
  Type_Volume,
  Type_Curve,
  Type_Light,
  Type_Mesh,
  Type_End
};

struct Entry {
  int type;
  int index;
};

/* the global scene data */
static struct Scene *the_scene = NULL;

static struct Scene *get_scene()
{
  return the_scene;
}

static void set_scene(struct Scene *scene)
{
  the_scene = scene;
}

/* ID map for link an ID to another ID */
struct IDmapEntry {
  ID key, value;
};

struct IDmap {
  int entry_count;
  struct IDmapEntry entry[1024];
};

static struct IDmap primset_to_accel = {0, {{0, 0}}};

static void push_idmap_endtry(ID key, ID value)
{
  const int index = primset_to_accel.entry_count;

  /* TODO check count */
  if (index == 1024)
    return;

  primset_to_accel.entry[index].key = key;
  primset_to_accel.entry[index].value = value;
  primset_to_accel.entry_count++;
}

static ID find_accelerator(ID primset)
{
  int i;
  for (i = 0; i < primset_to_accel.entry_count; i++) {
    const struct IDmapEntry *stored_entry = &primset_to_accel.entry[i];

    if (stored_entry->key == primset) {
      return stored_entry->value;
    }
  }
  return SI_BADID;
}

/* the global error code */
static int si_errno = SI_ERR_NONE;

static int is_valid_type(int type);
static ID encode_id(int type, int index);
static struct Entry decode_id(ID id);
static int create_implicit_groups(void);
static void compute_objects_bounds(void);
static void set_errno(int err_no);
static Status status_of_error(int err);

static int set_property(const struct Entry *entry,
    const char *name, const struct PropertyValue *value);

/* property list description */
#include "property_list_include.c"

/* Error interfaces */
int SiGetErrorNo(void)
{
  return si_errno;
}

/* Plugin interfaces */
Status SiOpenPlugin(const char *filename)
{
  if (ScnOpenPlugin(get_scene(), filename) == NULL) {
    /* TODO make a mapping function */
    switch (PlgGetErrorNo()) {
    case PLG_ERR_PLUGIN_NOT_FOUND:
      set_errno(SI_ERR_PLUGIN_NOT_FOUND);
      break;
    case PLG_ERR_INIT_PLUGIN_FUNC_NOT_EXIST:
      set_errno(SI_ERR_INIT_PLUGIN_FUNC_NOT_EXIST);
      break;
    case PLG_ERR_INIT_PLUGIN_FUNC_FAIL:
      set_errno(SI_ERR_INIT_PLUGIN_FUNC_FAIL);
      break;
    case PLG_ERR_BAD_PLUGIN_INFO:
      set_errno(SI_ERR_BAD_PLUGIN_INFO);
      break;
    case PLG_ERR_NO_MEMORY:
      set_errno(SI_ERR_NO_MEMORY);
      break;
    case PLG_ERR_CLOSE_PLUGIN_FAIL:
      set_errno(SI_ERR_CLOSE_PLUGIN_FAIL);
      break;
    default:
      break;
    }
    return SI_FAIL;
  }

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

/* Scene interfaces */
Status SiOpenScene(void)
{
  set_scene(ScnNew());

  if (get_scene() == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_FAIL;
  }

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

Status SiCloseScene(void)
{
  ScnFree(get_scene());
  set_scene(NULL);

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

Status SiRenderScene(ID renderer)
{
  int err = 0;

  compute_objects_bounds();

  err = create_implicit_groups();
  if (err) {
    /* TODO error handling */
    return SI_FAIL;
  }

  /* TODO fix hard-coded renderer index */
  err = RdrRender(ScnGetRenderer(get_scene(), 0));
  if (err) {
    /* TODO error handling */
    return SI_FAIL;
  }

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

Status SiSaveFrameBuffer(ID framebuffer, const char *filename)
{
  const struct Entry entry = decode_id(framebuffer);
  struct FrameBuffer *framebuffer_ptr = NULL;
  int err = 0;

  if (entry.type != Type_FrameBuffer)
    return SI_FAIL;

  framebuffer_ptr = ScnGetFrameBuffer(get_scene(), entry.index);
  if (framebuffer_ptr == NULL)
    return SI_FAIL;

  err = FbSaveCroppedData(framebuffer_ptr, filename);
  if (err) {
    /* TODO error handling */
    return SI_FAIL;
  }

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

Status SiRunProcedure(ID procedure)
{
  const struct Entry entry = decode_id(procedure);
  struct Procedure *procedure_ptr = NULL;
  int err = 0;

  if (entry.type != Type_Procedure)
    return SI_FAIL;

  procedure_ptr = ScnGetProcedure(get_scene(), entry.index);
  if (procedure_ptr == NULL)
    return SI_FAIL;

  err = PrcRun(procedure_ptr);
  if (err) {
    /* TODO error handling */
    return SI_FAIL;
  }

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

Status SiAddObjectToGroup(ID group, ID object)
{
  struct ObjectGroup *group_ptr = NULL;
  struct ObjectInstance *object_ptr = NULL;

  {
    const struct Entry entry = decode_id(group);
    if (entry.type != Type_ObjectGroup)
      return SI_FAIL;

    group_ptr = ScnGetObjectGroup(get_scene(), entry.index);
    if (group_ptr == NULL)
      return SI_FAIL;
  }
  {
    const struct Entry entry = decode_id(object);
    if (entry.type != Type_ObjectInstance)
      return SI_FAIL;

    object_ptr = ScnGetObjectInstance(get_scene(), entry.index);
    if (object_ptr == NULL)
      return SI_FAIL;
  }

  ObjGroupAdd(group_ptr, object_ptr);

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

ID SiNewObjectInstance(ID primset_id)
{
  const ID accel_id = find_accelerator(primset_id);
  const struct Entry entry = decode_id(accel_id);

  if (entry.type == Type_Accelerator) {
    struct ObjectInstance *object = NULL;
    struct Accelerator *acc = NULL;
    int err = 0;

    acc = ScnGetAccelerator(get_scene(), entry.index);
    if (acc == NULL) {
      set_errno(SI_ERR_BADTYPE);
      return SI_BADID;
    }

    object = ScnNewObjectInstance(get_scene());
    if (object == NULL) {
      set_errno(SI_ERR_NO_MEMORY);
      return SI_BADID;
    }

    err = ObjSetSurface(object, acc);
    if (err) {
      set_errno(SI_ERR_FAILNEW);
      return SI_BADID;
    }
  }
  else if (entry.type == Type_Volume) {
    struct ObjectInstance *object = NULL;
    struct Volume *volume = NULL;
    int err = 0;

    volume = ScnGetVolume(get_scene(), entry.index);
    if (volume == NULL) {
      set_errno(SI_ERR_BADTYPE);
      return SI_BADID;
    }

    object = ScnNewObjectInstance(get_scene());
    if (object == NULL) {
      set_errno(SI_ERR_NO_MEMORY);
      return SI_BADID;
    }

    err = ObjSetVolume(object, volume);
    if (err) {
      set_errno(SI_ERR_FAILNEW);
      return SI_BADID;
    }
  }
  else {
    set_errno(SI_ERR_BADTYPE);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_ObjectInstance, GET_LAST_ADDED_ID(ObjectInstance));
}

ID SiNewFrameBuffer(const char *arg)
{
  if (ScnNewFrameBuffer(get_scene()) == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_FrameBuffer, GET_LAST_ADDED_ID(FrameBuffer));
}

ID SiNewObjectGroup(void)
{
  if (ScnNewObjectGroup(get_scene()) == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_ObjectGroup, GET_LAST_ADDED_ID(ObjectGroup));
}

ID SiNewPointCloud(const char *filename)
{
  struct PointCloud *ptc = NULL;
  struct Accelerator *acc = NULL;
  struct PrimitiveSet primset;

  ID ptc_id = SI_BADID;
  ID accel_id = SI_BADID;

  ptc = ScnNewPointCloud(get_scene());
  if (ptc == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  /*
  if (strcmp(filename, "null") == 0) {
    MshClear(ptc);
  } else {
  */
    if (PtcLoadFile(ptc, filename)) {
      set_errno(SI_ERR_FAILLOAD);
      return SI_BADID;
    }
  /*
  }
  */

  acc = ScnNewAccelerator(get_scene(), ACC_GRID);
  if (acc == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  PtcGetPrimitiveSet(ptc, &primset);
  AccSetPrimitiveSet(acc, &primset);

  ptc_id = encode_id(Type_PointCloud, GET_LAST_ADDED_ID(PointCloud));
  accel_id = encode_id(Type_Accelerator, GET_LAST_ADDED_ID(Accelerator));
  push_idmap_endtry(ptc_id, accel_id);

  set_errno(SI_ERR_NONE);
  return ptc_id;
}

ID SiNewTurbulence(void)
{
  if (ScnNewTurbulence(get_scene()) == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Turbulence, GET_LAST_ADDED_ID(Turbulence));
}

ID SiNewProcedure(const char *plugin_name)
{
  struct Plugin **plugins = ScnGetPluginList(get_scene());
  struct Plugin *found = NULL;
  const int N = (int) ScnGetPluginCount(get_scene());
  int i = 0;

  for (i = 0; i < N; i++) {
    if (strcmp(plugin_name, PlgGetName(plugins[i])) == 0) {
      found = plugins[i];
      break;
    }
  }
  if (found == NULL) {
    set_errno(SI_ERR_FAILNEW);
    return SI_BADID;
  }
  if (ScnNewProcedure(get_scene(), found) == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Procedure, GET_LAST_ADDED_ID(Procedure));
}

ID SiNewRenderer(void)
{
  if (ScnNewRenderer(get_scene()) == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Renderer, GET_LAST_ADDED_ID(Renderer));
}

ID SiNewTexture(const char *filename)
{
  struct Texture *tex = ScnNewTexture(get_scene());

  if (tex == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  if (TexLoadFile(tex, filename)) {
    set_errno(SI_ERR_FAILLOAD);
    return SI_FAIL;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Texture, GET_LAST_ADDED_ID(Texture));
}

ID SiNewCamera(const char *arg)
{
  if (ScnNewCamera(get_scene(), arg) == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Camera, GET_LAST_ADDED_ID(Camera));
}

ID SiNewShader(const char *plugin_name)
{
  struct Plugin **plugins = ScnGetPluginList(get_scene());
  struct Plugin *found = NULL;
  const int N = (int) ScnGetPluginCount(get_scene());
  int i = 0;

  for (i = 0; i < N; i++) {
    if (strcmp(plugin_name, PlgGetName(plugins[i])) == 0) {
      found = plugins[i];
      break;
    }
  }
  if (found == NULL) {
    set_errno(SI_ERR_FAILNEW);
    return SI_BADID;
  }
  if (ScnNewShader(get_scene(), found) == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Shader, GET_LAST_ADDED_ID(Shader));
}

ID SiNewVolume(void)
{
  struct Volume *volume = ScnNewVolume(get_scene());
  ID volume_id = SI_BADID;

  if (volume == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  volume_id = encode_id(Type_Volume, GET_LAST_ADDED_ID(Volume));
  /* volume id should map to itself because it doesn't need accelerator */
  push_idmap_endtry(volume_id, volume_id);

  set_errno(SI_ERR_NONE);
  return volume_id;
}

ID SiNewCurve(const char *filename)
{
  struct Curve *curve = NULL;
  struct Accelerator *acc = NULL;
  struct PrimitiveSet primset;

  ID curve_id = SI_BADID;
  ID accel_id = SI_BADID;

  curve = ScnNewCurve(get_scene());
  if (curve == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  if (CrvLoadFile(curve, filename)) {
    set_errno(SI_ERR_FAILLOAD);
    return SI_BADID;
  }

  acc = ScnNewAccelerator(get_scene(), ACC_GRID);
  if (acc == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  CrvGetPrimitiveSet(curve, &primset);
  AccSetPrimitiveSet(acc, &primset);

  curve_id = encode_id(Type_Curve, GET_LAST_ADDED_ID(Curve));
  accel_id = encode_id(Type_Accelerator, GET_LAST_ADDED_ID(Accelerator));
  push_idmap_endtry(curve_id, accel_id);

  set_errno(SI_ERR_NONE);
  return curve_id;
}

ID SiNewLight(int light_type)
{
  int type = 0;
  switch (light_type) {
  case SI_POINT_LIGHT:
    type = LGT_POINT;
    break;
  case SI_GRID_LIGHT:
    type = LGT_GRID;
    break;
  case SI_SPHERE_LIGHT:
    type = LGT_SPHERE;
    break;
  case SI_DOME_LIGHT:
    type = LGT_DOME;
    break;
  default:
    type = LGT_POINT;
    break;
  };

  if (ScnNewLight(get_scene(), type) == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Light, GET_LAST_ADDED_ID(Light));
}

ID SiNewMesh(const char *filename)
{
  struct Mesh *mesh = NULL;
  struct Accelerator *acc = NULL;
  struct PrimitiveSet primset;

  ID mesh_id = SI_BADID;
  ID accel_id = SI_BADID;

  mesh = ScnNewMesh(get_scene());
  if (mesh == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  if (strcmp(filename, "null") == 0) {
    MshClear(mesh);
  } else {
    if (MshLoadFile(mesh, filename)) {
      set_errno(SI_ERR_FAILLOAD);
      return SI_BADID;
    }
  }

  acc = ScnNewAccelerator(get_scene(), ACC_GRID);
  if (acc == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  MshGetPrimitiveSet(mesh, &primset);
  AccSetPrimitiveSet(acc, &primset);

  mesh_id = encode_id(Type_Mesh, GET_LAST_ADDED_ID(Mesh));
  accel_id = encode_id(Type_Accelerator, GET_LAST_ADDED_ID(Accelerator));
  push_idmap_endtry(mesh_id, accel_id);

  set_errno(SI_ERR_NONE);
  return mesh_id;
}

Status SiAssignShader(ID object, ID shader)
{
  struct ObjectInstance *object_ptr = NULL;
  struct Shader *shader_ptr = NULL;
  {
    const struct Entry entry = decode_id(object);

    if (entry.type != Type_ObjectInstance)
      return SI_FAIL;

    object_ptr = ScnGetObjectInstance(get_scene(), entry.index);
    if (object_ptr == NULL)
      return SI_FAIL;
  }
  {
    const struct Entry entry = decode_id(shader);

    if (entry.type != Type_Shader)
      return SI_FAIL;

    shader_ptr = ScnGetShader(get_scene(), entry.index);
    if (shader_ptr == NULL)
      return SI_FAIL;
  }

  ObjSetShader(object_ptr, shader_ptr);
  return SI_SUCCESS;
}

Status SiAssignObjectGroup(ID id, const char *name, ID group)
{
  const struct Entry entry = decode_id(id);
  const struct Entry group_ent = decode_id(group);
  struct PropertyValue value = InitPropValue();
  struct ObjectGroup *group_ptr = NULL;
  int err = 0;

  if (group_ent.type != Type_ObjectGroup)
    return SI_FAIL;

  group_ptr = ScnGetObjectGroup(get_scene(), group_ent.index);
  if (group_ptr == NULL)
    return SI_FAIL;

  value = PropObjectGroup(group_ptr);
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiAssignTexture(ID id, const char *name, ID texture)
{
  const struct Entry entry = decode_id(id);
  const struct Entry texture_ent = decode_id(texture);
  struct PropertyValue value = InitPropValue();
  struct Texture *texture_ptr = NULL;
  int err = 0;

  if (texture_ent.type != Type_Texture)
    return SI_FAIL;

  texture_ptr = ScnGetTexture(get_scene(), texture_ent.index);
  if (texture_ptr == NULL)
    return SI_FAIL;

  value = PropTexture(texture_ptr);
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiAssignCamera(ID renderer, ID camera)
{
  struct Renderer *renderer_ptr = NULL;
  struct Camera *camera_ptr = NULL;
  {
    const struct Entry entry = decode_id(renderer);

    if (entry.type != Type_Renderer)
      return SI_FAIL;

    renderer_ptr = ScnGetRenderer(get_scene(), entry.index);
    if (renderer_ptr == NULL)
      return SI_FAIL;
  }
  {
    const struct Entry entry = decode_id(camera);

    if (entry.type != Type_Camera)
      return SI_FAIL;

    camera_ptr = ScnGetCamera(get_scene(), entry.index);
    if (camera_ptr == NULL)
      return SI_FAIL;
  }

  RdrSetCamera(renderer_ptr, camera_ptr);
  return SI_SUCCESS;
}

Status SiAssignFrameBuffer(ID renderer, ID framebuffer)
{
  struct Renderer *renderer_ptr = NULL;
  struct FrameBuffer *framebuffer_ptr = NULL;
  {
    const struct Entry entry = decode_id(renderer);

    if (entry.type != Type_Renderer)
      return SI_FAIL;

    renderer_ptr = ScnGetRenderer(get_scene(), entry.index);
    if (renderer_ptr == NULL)
      return SI_FAIL;
  }
  {
    const struct Entry entry = decode_id(framebuffer);

    if (entry.type != Type_FrameBuffer)
      return SI_FAIL;

    framebuffer_ptr = ScnGetFrameBuffer(get_scene(), entry.index);
    if (framebuffer_ptr == NULL)
      return SI_FAIL;
  }

  RdrSetFrameBuffers(renderer_ptr, framebuffer_ptr);
  return SI_SUCCESS;
}

Status SiSetProperty1(ID id, const char *name, double v0)
{
  const struct Entry entry = decode_id(id);
  const struct PropertyValue value = PropScalar(v0);
  const int err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiSetProperty2(ID id, const char *name, double v0, double v1)
{
  const struct Entry entry = decode_id(id);
  const struct PropertyValue value = PropVector2(v0, v1);
  const int err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiSetProperty3(ID id, const char *name, double v0, double v1, double v2)
{
  const struct Entry entry = decode_id(id);
  const struct PropertyValue value = PropVector3(v0, v1, v2);
  const int err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiSetProperty4(ID id, const char *name, double v0, double v1, double v2, double v3)
{
  const struct Entry entry = decode_id(id);
  const struct PropertyValue value = PropVector4(v0, v1, v2, v3);
  const int err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiSetStringProperty(ID id, const char *name, const char *string)
{
  const struct Entry entry = decode_id(id);
  const struct PropertyValue value = PropString(string);
  const int err = set_property(&entry, name, &value);

  return status_of_error(err);
}

/* time variable property */
Status SiSetSampleProperty3(ID id, const char *name, double v0, double v1, double v2, double time)
{
  const struct Entry entry = decode_id(id);
  struct PropertyValue value = PropVector3(v0, v1, v2);
  int err = 0;

  value.time = time;
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiAssignTurbulence(ID id, const char *name, ID turbulence)
{
  const struct Entry entry = decode_id(id);
  const struct Entry turbulence_ent = decode_id(turbulence);
  struct PropertyValue value = InitPropValue();
  struct Turbulence *turbulence_ptr = NULL;
  int err = 0;

  if (turbulence_ent.type != Type_Turbulence)
    return SI_FAIL;

  turbulence_ptr = ScnGetTurbulence(get_scene(), turbulence_ent.index);
  if (turbulence_ptr == NULL)
    return SI_FAIL;

  value = PropTurbulence(turbulence_ptr);
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiAssignVolume(ID id, const char *name, ID volume)
{
  const struct Entry entry = decode_id(id);
  const struct Entry volume_ent = decode_id(volume);
  struct PropertyValue value = InitPropValue();
  struct Volume *volume_ptr = NULL;
  int err = 0;

  if (volume_ent.type != Type_Volume)
    return SI_FAIL;

  volume_ptr = ScnGetVolume(get_scene(), volume_ent.index);
  if (volume_ptr == NULL)
    return SI_FAIL;

  value = PropVolume(volume_ptr);
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiAssignMesh(ID id, const char *name, ID mesh)
{
  const struct Entry entry = decode_id(id);
  const struct Entry mesh_ent = decode_id(mesh);
  struct PropertyValue value = InitPropValue();
  struct Mesh *mesh_ptr = NULL;
  int err = 0;

  if (mesh_ent.type != Type_Mesh)
    return SI_FAIL;

  mesh_ptr = ScnGetMesh(get_scene(), mesh_ent.index);
  if (mesh_ptr == NULL)
    return SI_FAIL;

  value = PropMesh(mesh_ptr);
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiSetInterruptCallback(ID id, InterruptCallback interrupt, void *data)
{
  const struct Entry entry = decode_id(id);

  if (entry.type == Type_Renderer) {
    struct Renderer *renderer_ptr = ScnGetRenderer(get_scene(), entry.index);
    RdrSetInterruptCallback(renderer_ptr, interrupt, data);
    return SI_SUCCESS;
  } else {
    return SI_FAIL;
  }
}

Status SiGetPropertyList(const char *type_name,
    const char ***property_types,
    const char ***property_names,
    int *property_count)
{
  return get_property_list(type_name, property_types, property_names, property_count);
}

static int is_valid_type(int type)
{
  return type > Type_Begin && type < Type_End;
}

static ID encode_id(int type, int index)
{
  return TYPE_ID_OFFSET * type + index;
}

static struct Entry decode_id(ID id)
{
  struct Entry entry = {-1, -1};
  const int type = id / TYPE_ID_OFFSET;

  if (!is_valid_type(type))
    return entry;

  entry.type = type;
  entry.index = id - (type * TYPE_ID_OFFSET);

  return entry;
}

static int create_implicit_groups(void)
{
  struct ObjectGroup *all_objects = NULL;
  struct Renderer *renderer = NULL;
  size_t i;

  all_objects = ScnNewObjectGroup(get_scene());
  if (all_objects == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_FAIL;
  }

  for (i = 0; i < ScnGetObjectInstanceCount(get_scene()); i++) {
    struct ObjectInstance *obj = ScnGetObjectInstance(get_scene(), i);
    ObjGroupAdd(all_objects, obj);
  }

  /* Preparing ObjectInstance */
  for (i = 0; i < ScnGetObjectInstanceCount(get_scene()); i++) {
    struct ObjectInstance *obj = ScnGetObjectInstance(get_scene(), i);
    const struct Light **lightlist = (const struct Light **) ScnGetLightList(get_scene());
    int nlights = ScnGetLightCount(get_scene());
    ObjSetLightList(obj, lightlist, nlights);

    if (ObjGetReflectTarget(obj) == NULL)
      ObjSetReflectTarget(obj, all_objects);

    if (ObjGetRefractTarget(obj) == NULL)
      ObjSetRefractTarget(obj, all_objects);

    if (ObjGetShadowTarget(obj) == NULL)
      ObjSetShadowTarget(obj, all_objects);

    {
      /* self hit group */
      struct ObjectGroup *self_group = ScnNewObjectGroup(get_scene());
      if (self_group == NULL) {
        /* TODO error handling */
      }
      ObjGroupAdd(self_group, obj);
      ObjSetSelfHitTarget(obj, self_group);
    }
  }

  renderer = ScnGetRenderer(get_scene(), 0);
  RdrSetTargetObjects(renderer, all_objects);
  {
    const int nlights = ScnGetLightCount(get_scene());
    struct Light **lightlist = ScnGetLightList(get_scene());
    RdrSetTargetLights(renderer, lightlist, nlights);
  }

  return SI_SUCCESS;
}

static void compute_objects_bounds(void)
{
  int N = 0;
  int i;

  N = ScnGetAcceleratorCount(get_scene());
  for (i = 0; i < N; i++) {
    struct Accelerator *acc = ScnGetAccelerator(get_scene(), i);
    AccComputeBounds(acc);
  }

  N = ScnGetObjectInstanceCount(get_scene());
  for (i = 0; i < N; i++) {
    struct ObjectInstance *obj = ScnGetObjectInstance(get_scene(), i);
    ObjComputeBounds(obj);
  }

  N = ScnGetObjectGroupCount(get_scene());
  for (i = 0; i < N; i++) {
    struct ObjectGroup *grp = ScnGetObjectGroup(get_scene(), i);
    ObjGroupComputeBounds(grp);
  }
}

static void set_errno(int err_no)
{
  si_errno = err_no;
}

static Status status_of_error(int err)
{
  if (err)
    return SI_FAIL;
  else
    return SI_SUCCESS;
}

/* TODO refactoring in terms of variable names */
static int find_and_set_property(void *self, const struct Property *src_props,
    const char *prop_name, const struct PropertyValue *src_data)
{
  const struct Property *dst_prop = PropFind(src_props, src_data->type, prop_name);
  if (dst_prop == NULL)
    return -1;

  if (self == NULL)
    return -1;

  assert(dst_prop->SetProperty != NULL);
  return dst_prop->SetProperty(self, src_data);
}

static int set_property(const struct Entry *entry,
    const char *name, const struct PropertyValue *value)
{
  const struct Property *src_props = NULL;
  void *dst_object = NULL;

  /* procedure and shader type properties */
  if (entry->type == Type_Procedure) {
    struct Procedure *procedure = ScnGetProcedure(get_scene(), entry->index);
    if (procedure == NULL)
      return SI_FAIL;

    return PrcSetProperty(procedure, name, value);
  }
  else if (entry->type == Type_Shader) {
    struct Shader *shader = ScnGetShader(get_scene(), entry->index);
    if (shader == NULL)
      return SI_FAIL;

    return ShdSetProperty(shader, name, value);
  }

  /* builtin type properties */
  dst_object = get_builtin_type_entry(get_scene(), entry);
  src_props  = get_builtin_type_properties(entry->type);

  assert(src_props && "Some types are not implemented yet");

  if (dst_object == NULL)
    return -1;

  return find_and_set_property(dst_object, src_props, name, value);
}

