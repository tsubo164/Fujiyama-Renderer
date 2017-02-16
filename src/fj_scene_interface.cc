// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_scene_interface.h"
#include "fj_volume_accelerator.h"
#include "fj_framebuffer_io.h"
#include "fj_point_cloud_io.h"
#include "fj_primitive_set.h"
#include "fj_multi_thread.h"
#include "fj_curve_io.h"
#include "fj_mesh_io.h"
#include "fj_shader.h"
#include "fj_scene.h"
#include "fj_timer.h"
#include "fj_box.h"

#include <map>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#define GET_LAST_ADDED_ID(Type) (get_scene()->Get##Type##Count() - 1)

namespace fj {

static const int TYPE_ID_OFFSET = 10000000;

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

class Entry {
public:
  Entry() : type(-1), index(-1) {}
  ~Entry() {}

public:
  int type;
  int index;
};

/* the global scene data */
static Scene *the_scene = NULL;

static Scene *get_scene()
{
  return the_scene;
}

static void set_scene(Scene *scene)
{
  the_scene = scene;
}

// binding ID to ID
typedef std::map<ID,ID> IDMap;
IDMap object_to_primset;
IDMap primset_to_accelerator;

static void push_idmap_entry(IDMap &map, ID key, ID value)
{
  IDMap::const_iterator it = map.find(key);
  if (it == map.end()) {
    map[key] = value;
  }
}

static ID find_idmap_entry(IDMap &map, ID key)
{
  IDMap::const_iterator it = map.find(key);
  if (it != map.end()) {
    return it->second;
  } else {
    return SI_BADID;
  }
}

static void bind_primset_to_accelerator(ID primset, ID accel)
{
  push_idmap_entry(primset_to_accelerator, primset, accel);
}

static ID find_accelerator_from(ID primset)
{
  return find_idmap_entry(primset_to_accelerator, primset);
}

static void bind_object_to_primset(ID object, ID primset)
{
  push_idmap_entry(object_to_primset, object, primset);
}

static ID find_primset_from(ID object)
{
  return find_idmap_entry(object_to_primset, object);
}

/* the global error code */
static int si_errno = SI_ERR_NONE;

static int is_valid_type(int type);
static ID encode_id(int type, int index);
static Entry decode_id(ID id);
static int prepare_render(const Renderer *renderer);
static void set_errno(int err_no);
static Status status_of_error(int err);

static int set_property(const Entry *entry,
    const char *name, const PropertyValue *value);

/* property list description */
#include "internal/fj_property_list_include.cc"

/* Error interfaces */
int SiGetErrorNo(void)
{
  return si_errno;
}

/* Plugin interfaces */
Status SiOpenPlugin(const char *filename)
{
  if (get_scene()->OpenPlugin(filename) == NULL) {
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
  set_scene(new Scene());

  if (get_scene() == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_FAIL;
  }

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

Status SiCloseScene(void)
{
  delete get_scene();
  set_scene(NULL);

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

Status SiRenderScene(ID renderer)
{
  const Entry entry = decode_id(renderer);
  Renderer *renderer_ptr = NULL;
  int err = 0;

  if (entry.type != Type_Renderer) {
    /* TODO error handling */
    return SI_FAIL;
  }

  renderer_ptr = get_scene()->GetRenderer(entry.index);
  if (renderer_ptr == NULL) {
    /* TODO error handling */
    return SI_FAIL;
  }

  err = prepare_render(renderer_ptr);
  if (err) {
    /* TODO error handling */
    return SI_FAIL;
  }

  err = renderer_ptr->RenderScene();
  if (err) {
    /* TODO error handling */
    return SI_FAIL;
  }

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

Status SiSaveFrameBuffer(ID framebuffer, const char *filename)
{
  const Entry entry = decode_id(framebuffer);
  FrameBuffer *framebuffer_ptr = NULL;
  int err = 0;

  if (entry.type != Type_FrameBuffer) {
    /* TODO error handling */
    return SI_FAIL;
  }

  framebuffer_ptr = get_scene()->GetFrameBuffer(entry.index);
  if (framebuffer_ptr == NULL) {
    /* TODO error handling */
    return SI_FAIL;
  }

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
  const Entry entry = decode_id(procedure);
  Procedure *procedure_ptr = NULL;
  int err = 0;

  if (entry.type != Type_Procedure)
    return SI_FAIL;

  procedure_ptr = get_scene()->GetProcedure(entry.index);
  if (procedure_ptr == NULL)
    return SI_FAIL;

  err = procedure_ptr->Run();
  if (err) {
    /* TODO error handling */
    return SI_FAIL;
  }

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

Status SiAddObjectToGroup(ID group, ID object)
{
  ObjectGroup *group_ptr = NULL;
  ObjectInstance *object_ptr = NULL;

  {
    const Entry entry = decode_id(group);
    if (entry.type != Type_ObjectGroup)
      return SI_FAIL;

    group_ptr = get_scene()->GetObjectGroup(entry.index);
    if (group_ptr == NULL)
      return SI_FAIL;
  }
  {
    const Entry entry = decode_id(object);
    if (entry.type != Type_ObjectInstance)
      return SI_FAIL;

    object_ptr = get_scene()->GetObjectInstance(entry.index);
    if (object_ptr == NULL)
      return SI_FAIL;
  }

  group_ptr->AddObject(object_ptr);

  set_errno(SI_ERR_NONE);
  return SI_SUCCESS;
}

ID SiNewObjectInstance(ID primset)
{
  const ID accel_id = find_accelerator_from(primset);
  const Entry entry = decode_id(accel_id);

  if (entry.type == Type_Accelerator) {
    ObjectInstance *object = NULL;
    Accelerator *acc = NULL;
    int err = 0;

    acc = get_scene()->GetAccelerator(entry.index);
    if (acc == NULL) {
      set_errno(SI_ERR_BADTYPE);
      return SI_BADID;
    }

    object = get_scene()->NewObjectInstance();
    if (object == NULL) {
      set_errno(SI_ERR_NO_MEMORY);
      return SI_BADID;
    }

    err = object->SetSurface(acc);
    if (err) {
      set_errno(SI_ERR_FAILNEW);
      return SI_BADID;
    }
    PropSetAllDefaultValues(object, get_builtin_type_property_list(Type_ObjectInstance));
  }
  else if (entry.type == Type_Volume) {
    ObjectInstance *object = NULL;
    Volume *volume = NULL;
    int err = 0;

    volume = get_scene()->GetVolume(entry.index);
    if (volume == NULL) {
      set_errno(SI_ERR_BADTYPE);
      return SI_BADID;
    }

    object = get_scene()->NewObjectInstance();
    if (object == NULL) {
      set_errno(SI_ERR_NO_MEMORY);
      return SI_BADID;
    }

    err = object->SetVolume(volume);
    if (err) {
      set_errno(SI_ERR_FAILNEW);
      return SI_BADID;
    }
    PropSetAllDefaultValues(object, get_builtin_type_property_list(Type_ObjectInstance));
  }
  else {
    set_errno(SI_ERR_BADTYPE);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);

  const ID obj_id = encode_id(Type_ObjectInstance, GET_LAST_ADDED_ID(ObjectInstance));
  bind_object_to_primset(obj_id, primset);

  return obj_id;
}

ID SiNewFrameBuffer(const char *arg)
{
  if (get_scene()->NewFrameBuffer() == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_FrameBuffer, GET_LAST_ADDED_ID(FrameBuffer));
}

ID SiNewObjectGroup(void)
{
  if (get_scene()->NewObjectGroup() == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_ObjectGroup, GET_LAST_ADDED_ID(ObjectGroup));
}

ID SiNewPointCloud(const char *filename)
{
  PointCloud *ptc = NULL;
  Accelerator *acc = NULL;

  ID ptc_id = SI_BADID;
  ID accel_id = SI_BADID;

  ptc = get_scene()->NewPointCloud();
  if (ptc == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  /*
  if (strcmp(filename, "null") == 0) {
    MshClear(ptc);
  } else {
  */
    if (PtcLoadFile(*ptc, filename)) {
      set_errno(SI_ERR_FAILLOAD);
      return SI_BADID;
    }
  /*
  }
  */

  acc = get_scene()->NewAccelerator(ACC_GRID);
  if (acc == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  acc->SetPrimitiveSet(ptc);

  ptc_id = encode_id(Type_PointCloud, GET_LAST_ADDED_ID(PointCloud));
  accel_id = encode_id(Type_Accelerator, GET_LAST_ADDED_ID(Accelerator));
  bind_primset_to_accelerator(ptc_id, accel_id);

  set_errno(SI_ERR_NONE);
  return ptc_id;
}

ID SiNewTurbulence(void)
{
  Turbulence *turb = get_scene()->NewTurbulence();
  if (turb == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  PropSetAllDefaultValues(turb, get_builtin_type_property_list(Type_Turbulence));

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Turbulence, GET_LAST_ADDED_ID(Turbulence));
}

ID SiNewProcedure(const char *plugin_name)
{
  Plugin **plugins = get_scene()->GetPluginList();
  Plugin *found = NULL;
  const int N = (int) get_scene()->GetPluginCount();
  int i = 0;

  for (i = 0; i < N; i++) {
    if (strcmp(plugin_name, plugins[i]->GetName()) == 0) {
      found = plugins[i];
      break;
    }
  }
  if (found == NULL) {
    set_errno(SI_ERR_FAILNEW);
    return SI_BADID;
  }
  if (get_scene()->NewProcedure(found) == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Procedure, GET_LAST_ADDED_ID(Procedure));
}

ID SiNewRenderer(void)
{
  Renderer *renderer = get_scene()->NewRenderer();
  if (renderer == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  PropSetAllDefaultValues(renderer, get_builtin_type_property_list(Type_Renderer));

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Renderer, GET_LAST_ADDED_ID(Renderer));
}

ID SiNewTexture(const char *filename)
{
  Texture *tex = get_scene()->NewTexture();

  if (tex == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  if (tex->LoadFile(filename)) {
    set_errno(SI_ERR_FAILLOAD);
    return SI_FAIL;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Texture, GET_LAST_ADDED_ID(Texture));
}

ID SiNewCamera(const char *arg)
{
  Camera *camera = get_scene()->NewCamera(arg);
  if (camera == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  PropSetAllDefaultValues(camera, get_builtin_type_property_list(Type_Camera));

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Camera, GET_LAST_ADDED_ID(Camera));
}

ID SiNewShader(const char *plugin_name)
{
  Plugin **plugins = get_scene()->GetPluginList();
  Plugin *found = NULL;
  const int N = (int) get_scene()->GetPluginCount();
  int i = 0;

  for (i = 0; i < N; i++) {
    if (strcmp(plugin_name, plugins[i]->GetName()) == 0) {
      found = plugins[i];
      break;
    }
  }
  if (found == NULL) {
    set_errno(SI_ERR_FAILNEW);
    return SI_BADID;
  }
  if (get_scene()->NewShader(found) == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Shader, GET_LAST_ADDED_ID(Shader));
}

ID SiNewVolume(void)
{
  Volume *volume = get_scene()->NewVolume();
  ID volume_id = SI_BADID;

  if (volume == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  volume_id = encode_id(Type_Volume, GET_LAST_ADDED_ID(Volume));
  /* volume id should map to itself because it doesn't need accelerator */
  bind_primset_to_accelerator(volume_id, volume_id);

  PropSetAllDefaultValues(volume, get_builtin_type_property_list(Type_Volume));

  set_errno(SI_ERR_NONE);
  return volume_id;
}

ID SiNewCurve(const char *filename)
{
  Curve *curve = NULL;
  Accelerator *acc = NULL;

  ID curve_id = SI_BADID;
  ID accel_id = SI_BADID;

  curve = get_scene()->NewCurve();
  if (curve == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  if (CrvLoadFile(curve, filename)) {
    set_errno(SI_ERR_FAILLOAD);
    return SI_BADID;
  }

  acc = get_scene()->NewAccelerator(ACC_GRID);
  if (acc == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  acc->SetPrimitiveSet(curve);

  curve_id = encode_id(Type_Curve, GET_LAST_ADDED_ID(Curve));
  accel_id = encode_id(Type_Accelerator, GET_LAST_ADDED_ID(Accelerator));
  bind_primset_to_accelerator(curve_id, accel_id);

  set_errno(SI_ERR_NONE);
  return curve_id;
}

ID SiNewLight(int light_type)
{
  Light *light = NULL;
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

  light = get_scene()->NewLight(type);
  if (light == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  PropSetAllDefaultValues(light, get_builtin_type_property_list(Type_Light));

  set_errno(SI_ERR_NONE);
  return encode_id(Type_Light, GET_LAST_ADDED_ID(Light));
}

ID SiNewMesh(const char *filename)
{
  Mesh *mesh = NULL;
  Accelerator *acc = NULL;

  ID mesh_id = SI_BADID;
  ID accel_id = SI_BADID;

  mesh = get_scene()->NewMesh();
  if (mesh == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }
  if (strcmp(filename, "null") == 0) {
    mesh->Clear();
  } else {
    if (MshLoadFile(mesh, filename)) {
      set_errno(SI_ERR_FAILLOAD);
      return SI_BADID;
    }
  }

  acc = get_scene()->NewAccelerator(ACC_GRID);
  if (acc == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_BADID;
  }

  acc->SetPrimitiveSet(mesh);

  mesh_id = encode_id(Type_Mesh, GET_LAST_ADDED_ID(Mesh));
  accel_id = encode_id(Type_Accelerator, GET_LAST_ADDED_ID(Accelerator));
  bind_primset_to_accelerator(mesh_id, accel_id);

  set_errno(SI_ERR_NONE);
  return mesh_id;
}

Status SiAssignShader(ID object, const char *shading_group, ID shader)
{
  ObjectInstance *object_ptr = NULL;
  Shader *shader_ptr = NULL;
  int shading_group_id = 0;

  {
    const Entry entry = decode_id(object);

    if (entry.type != Type_ObjectInstance)
      return SI_FAIL;

    object_ptr = get_scene()->GetObjectInstance(entry.index);
    if (object_ptr == NULL)
      return SI_FAIL;
  }
  {
    const Entry entry = decode_id(shader);

    if (entry.type != Type_Shader)
      return SI_FAIL;

    shader_ptr = get_scene()->GetShader(entry.index);
    if (shader_ptr == NULL)
      return SI_FAIL;
  }
  {
    const ID primset_id = find_primset_from(object);
    const Entry entry = decode_id(primset_id);
    const std::string shading_group_name(shading_group);

    if (entry.type == Type_Mesh) {
      const Mesh *mesh = get_scene()->GetMesh(entry.index);
      shading_group_id = mesh->LookupFaceGroup(shading_group_name);
    }

    if (shading_group_id == -1) {
      // TODO warn group not found
      shading_group_id = 0;
    }
  }

  object_ptr->SetShader(shader_ptr, shading_group_id);
  return SI_SUCCESS;
}

Status SiAssignObjectGroup(ID id, const char *name, ID group)
{
  const Entry entry = decode_id(id);
  const Entry group_ent = decode_id(group);
  PropertyValue value;
  ObjectGroup *group_ptr = NULL;
  int err = 0;

  if (group_ent.type != Type_ObjectGroup)
    return SI_FAIL;

  group_ptr = get_scene()->GetObjectGroup(group_ent.index);
  if (group_ptr == NULL)
    return SI_FAIL;

  value = PropObjectGroup(group_ptr);
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiAssignTexture(ID id, const char *name, ID texture)
{
  const Entry entry = decode_id(id);
  const Entry texture_ent = decode_id(texture);
  PropertyValue value;
  Texture *texture_ptr = NULL;
  int err = 0;

  if (texture_ent.type != Type_Texture)
    return SI_FAIL;

  texture_ptr = get_scene()->GetTexture(texture_ent.index);
  if (texture_ptr == NULL)
    return SI_FAIL;

  value = PropTexture(texture_ptr);
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiAssignCamera(ID renderer, ID camera)
{
  Renderer *renderer_ptr = NULL;
  Camera *camera_ptr = NULL;
  {
    const Entry entry = decode_id(renderer);

    if (entry.type != Type_Renderer)
      return SI_FAIL;

    renderer_ptr = get_scene()->GetRenderer(entry.index);
    if (renderer_ptr == NULL)
      return SI_FAIL;
  }
  {
    const Entry entry = decode_id(camera);

    if (entry.type != Type_Camera)
      return SI_FAIL;

    camera_ptr = get_scene()->GetCamera(entry.index);
    if (camera_ptr == NULL)
      return SI_FAIL;
  }

  renderer_ptr->SetCamera(camera_ptr);
  return SI_SUCCESS;
}

Status SiAssignFrameBuffer(ID renderer, ID framebuffer)
{
  Renderer *renderer_ptr = NULL;
  FrameBuffer *framebuffer_ptr = NULL;
  {
    const Entry entry = decode_id(renderer);

    if (entry.type != Type_Renderer)
      return SI_FAIL;

    renderer_ptr = get_scene()->GetRenderer(entry.index);
    if (renderer_ptr == NULL)
      return SI_FAIL;
  }
  {
    const Entry entry = decode_id(framebuffer);

    if (entry.type != Type_FrameBuffer)
      return SI_FAIL;

    framebuffer_ptr = get_scene()->GetFrameBuffer(entry.index);
    if (framebuffer_ptr == NULL)
      return SI_FAIL;
  }

  renderer_ptr->SetFrameBuffers(framebuffer_ptr);
  return SI_SUCCESS;
}

Status SiSetProperty1(ID id, const char *name, double v0)
{
  const Entry entry = decode_id(id);
  const PropertyValue value = PropScalar(v0);
  const int err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiSetProperty2(ID id, const char *name, double v0, double v1)
{
  const Entry entry = decode_id(id);
  const PropertyValue value = PropVector2(v0, v1);
  const int err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiSetProperty3(ID id, const char *name, double v0, double v1, double v2)
{
  const Entry entry = decode_id(id);
  const PropertyValue value = PropVector3(v0, v1, v2);
  const int err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiSetProperty4(ID id, const char *name, double v0, double v1, double v2, double v3)
{
  const Entry entry = decode_id(id);
  const PropertyValue value = PropVector4(v0, v1, v2, v3);
  const int err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiSetStringProperty(ID id, const char *name, const char *string)
{
  const Entry entry = decode_id(id);
  const PropertyValue value = PropString(string);
  const int err = set_property(&entry, name, &value);

  return status_of_error(err);
}

/* time variable property */
Status SiSetSampleProperty3(ID id, const char *name, double v0, double v1, double v2, double time)
{
  const Entry entry = decode_id(id);
  PropertyValue value = PropVector3(v0, v1, v2);
  int err = 0;

  value.time = time;
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiAssignTurbulence(ID id, const char *name, ID turbulence)
{
  const Entry entry = decode_id(id);
  const Entry turbulence_ent = decode_id(turbulence);
  PropertyValue value;
  Turbulence *turbulence_ptr = NULL;
  int err = 0;

  if (turbulence_ent.type != Type_Turbulence)
    return SI_FAIL;

  turbulence_ptr = get_scene()->GetTurbulence(turbulence_ent.index);
  if (turbulence_ptr == NULL)
    return SI_FAIL;

  value = PropTurbulence(turbulence_ptr);
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiAssignVolume(ID id, const char *name, ID volume)
{
  const Entry entry = decode_id(id);
  const Entry volume_ent = decode_id(volume);
  PropertyValue value;
  Volume *volume_ptr = NULL;
  int err = 0;

  if (volume_ent.type != Type_Volume)
    return SI_FAIL;

  volume_ptr = get_scene()->GetVolume(volume_ent.index);
  if (volume_ptr == NULL)
    return SI_FAIL;

  value = PropVolume(volume_ptr);
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiAssignMesh(ID id, const char *name, ID mesh)
{
  const Entry entry = decode_id(id);
  const Entry mesh_ent = decode_id(mesh);
  PropertyValue value;
  Mesh *mesh_ptr = NULL;
  int err = 0;

  if (mesh_ent.type != Type_Mesh)
    return SI_FAIL;

  mesh_ptr = get_scene()->GetMesh(mesh_ent.index);
  if (mesh_ptr == NULL)
    return SI_FAIL;

  value = PropMesh(mesh_ptr);
  err = set_property(&entry, name, &value);

  return status_of_error(err);
}

Status SiSetFrameReportCallback(ID id, void *data,
    FrameStartCallback frame_start,
    FrameAbortCallback frame_abort,
    FrameDoneCallback frame_done)
{
  const Entry entry = decode_id(id);

  if (entry.type == Type_Renderer) {
    Renderer *renderer_ptr = get_scene()->GetRenderer(entry.index);
    renderer_ptr->SetFrameReportCallback(
        data,
        frame_start,
        frame_abort,
        frame_done);
    return SI_SUCCESS;
  } else {
    return SI_FAIL;
  }
}

Status SiSetTileReportCallback(ID id, void *data,
    TileStartCallback tile_start,
    SampleDoneCallback sample_done,
    TileDoneCallback tile_done)
{
  const Entry entry = decode_id(id);

  if (entry.type == Type_Renderer) {
    Renderer *renderer_ptr = get_scene()->GetRenderer(entry.index);
    renderer_ptr->SetTileReportCallback(
        data,
        tile_start,
        sample_done,
        tile_done);
    return SI_SUCCESS;
  } else {
    return SI_FAIL;
  }
}

const Property *SiGetPropertyList(const char *type_name)
{
  return get_property_list(type_name);
}

static int is_valid_type(int type)
{
  return type > Type_Begin && type < Type_End;
}

static ID encode_id(int type, int index)
{
  return TYPE_ID_OFFSET * type + index;
}

static Entry decode_id(ID id)
{
  Entry entry;
  const int type = id / TYPE_ID_OFFSET;

  if (!is_valid_type(type))
    return entry;

  entry.type = type;
  entry.index = id - (type * TYPE_ID_OFFSET);

  return entry;
}

static int create_implicit_groups(void)
{
  ObjectGroup *all_objects = NULL;
  Renderer *renderer = NULL;
  int N = 0;
  int i;

  all_objects = get_scene()->NewObjectGroup();
  if (all_objects == NULL) {
    set_errno(SI_ERR_NO_MEMORY);
    return SI_FAIL;
  }

  N = get_scene()->GetObjectInstanceCount();
  for (i = 0; i < N; i++) {
    ObjectInstance *obj = get_scene()->GetObjectInstance(i);
    all_objects->AddObject(obj);
  }

  /* Preparing ObjectInstance */
  N = get_scene()->GetObjectInstanceCount();
  for (i = 0; i < N; i++) {
    ObjectInstance *obj = get_scene()->GetObjectInstance(i);
    const Light **lightlist = (const Light **) get_scene()->GetLightList();
    int nlights = get_scene()->GetLightCount();
    obj->SetLightList(lightlist, nlights);

    if (obj->GetReflectTarget() == NULL)
      obj->SetReflectTarget(all_objects);

    if (obj->GetRefractTarget() == NULL)
      obj->SetRefractTarget(all_objects);

    if (obj->GetShadowTarget() == NULL)
      obj->SetShadowTarget(all_objects);

    {
      /* self hit group */
      ObjectGroup *self_group = get_scene()->NewObjectGroup();
      if (self_group == NULL) {
        /* TODO error handling */
      }
      self_group->AddObject(obj);
      obj->SetSelfHitTarget(self_group);
    }
  }

  renderer = get_scene()->GetRenderer(0);
  renderer->SetTargetObjects(all_objects);
  {
    const int nlights = get_scene()->GetLightCount();
    Light **lightlist = get_scene()->GetLightList();
    renderer->SetTargetLights(lightlist, nlights);
  }

  return SI_SUCCESS;
}

static void compute_objects_bounds(void)
{
  int N = 0;
  int i;

  N = get_scene()->GetAcceleratorCount();
  for (i = 0; i < N; i++) {
    Accelerator *acc = get_scene()->GetAccelerator(i);
    acc->ComputeBounds();
  }

  N = get_scene()->GetObjectInstanceCount();
  for (i = 0; i < N; i++) {
    ObjectInstance *obj = get_scene()->GetObjectInstance(i);
    obj->ComputeBounds();
  }

  N = get_scene()->GetObjectGroupCount();
  for (i = 0; i < N; i++) {
    ObjectGroup *grp = get_scene()->GetObjectGroup(i);
    grp->ComputeBounds();
  }
}

static void build_accelerators(void)
{
  Timer timer;
  Elapse elapse;
  int NOBJTECTS = 0;
  int NGROUPS = 0;
  int i;

  NOBJTECTS = get_scene()->GetAcceleratorCount();
  NGROUPS = get_scene()->GetObjectGroupCount();

  printf("# Building Accelerators\n");
  printf("#   Accelerator Count: %d\n", NOBJTECTS + NGROUPS);
  timer.Start();

  for (i = 0; i < NOBJTECTS; i++) {
    Accelerator *acc = get_scene()->GetAccelerator(i);
    acc->Build();
  }

  for (i = 0; i < NGROUPS; i++) {
    ObjectGroup *grp = get_scene()->GetObjectGroup(i);
    Accelerator *mutable_acc = NULL;
    VolumeAccelerator *mutable_volume_acc = NULL;

    // TODO TRY TO AVOID MUTABLE
    mutable_acc = (Accelerator *) grp->GetSurfaceAccelerator();
    mutable_volume_acc = (VolumeAccelerator *) grp->GetVolumeAccelerator();

    /* TODO come up with a better way */
    if (mutable_acc != NULL) {
      mutable_acc->Build();
    }
    if (mutable_volume_acc != NULL) {
      VolumeAccBuild(mutable_volume_acc);
    }
  }

  elapse = timer.GetElapse();
  printf("# Building Accelerators Done\n");
  printf("#   %dh %dm %ds\n\n", elapse.hour, elapse.min, elapse.sec);
}

static int prepare_render(const Renderer *renderer)
{
  int err = 0;

  printf("\n");

  compute_objects_bounds();

  /* TODO need err? */
  err = create_implicit_groups();
  if (err) {
    /* TODO error handling */
    return SI_FAIL;
  }

  build_accelerators();

  return 0;
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
static int find_and_set_property(void *self, const Property *src_props,
    const char *prop_name, const PropertyValue *src_data)
{
  const Property *dst_prop = PropFind(src_props, src_data->type, prop_name);
  if (dst_prop == NULL)
    return -1;

  if (self == NULL)
    return -1;

  assert(dst_prop->SetProperty != NULL);
  return dst_prop->SetProperty(self, src_data);
}

static int set_property(const Entry *entry,
    const char *name, const PropertyValue *value)
{
  const Property *src_props = NULL;
  void *dst_entry = NULL;

  /* procedure and shader type properties */
  if (entry->type == Type_Procedure) {
    Procedure *procedure = get_scene()->GetProcedure(entry->index);
    if (procedure == NULL)
      return SI_FAIL;

    return procedure->SetProperty(name, *value);
  }
  else if (entry->type == Type_Shader) {
    Shader *shader = get_scene()->GetShader(entry->index);
    if (shader == NULL)
      return SI_FAIL;

    return shader->SetPropertyValue(name, *value);
  }

  /* builtin type properties */
  dst_entry = get_builtin_type_entry(get_scene(), entry);
  src_props = get_builtin_type_property_list(entry->type);

  assert(src_props && "Some types are not implemented yet");

  if (dst_entry == NULL)
    return -1;

  return find_and_set_property(dst_entry, src_props, name, value);
}

} // namespace xxx
