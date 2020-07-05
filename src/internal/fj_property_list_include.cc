// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

// this file is inteded to be included in fj_scene_interface.c 
// defines all properties for builtin type and some helper functions

static int set_ObjectInstance_transform_order(void *self, const PropertyValue &value)
{
  // TODO error handling
  if (!XfmIsTransformOrder((int) value.vector[0]))
    return -1;

  ObjectInstance *obj = reinterpret_cast<ObjectInstance *>(self);
  obj->SetTransformOrder((int) value.vector[0]);
  return 0;
}

static int set_ObjectInstance_rotate_order(void *self, const PropertyValue &value)
{
  // TODO error handling
  if (!XfmIsRotateOrder((int) value.vector[0]))
    return -1;

  ObjectInstance *obj = reinterpret_cast<ObjectInstance *>(self);
  obj->SetRotateOrder((int) value.vector[0]);
  return 0;
}

static int set_ObjectInstance_translate(void *self, const PropertyValue &value)
{
  ObjectInstance *obj = reinterpret_cast<ObjectInstance *>(self);
  obj->SetTranslate(value.vector[0], value.vector[1], value.vector[2], value.time);
  return 0;
}

static int set_ObjectInstance_rotate(void *self, const PropertyValue &value)
{
  ObjectInstance *obj = reinterpret_cast<ObjectInstance *>(self);
  obj->SetRotate(value.vector[0], value.vector[1], value.vector[2], value.time);
  return 0;
}

static int set_ObjectInstance_scale(void *self, const PropertyValue &value)
{
  ObjectInstance *obj = reinterpret_cast<ObjectInstance *>(self);
  obj->SetScale(value.vector[0], value.vector[1], value.vector[2], value.time);
  return 0;
}

static int set_ObjectInstance_reflect_target(void *self, const PropertyValue &value)
{
  ObjectInstance *obj = reinterpret_cast<ObjectInstance *>(self);
  obj->SetReflectTarget(value.object_group);
  return 0;
}

static int set_ObjectInstance_refract_target(void *self, const PropertyValue &value)
{
  ObjectInstance *obj = reinterpret_cast<ObjectInstance *>(self);
  obj->SetRefractTarget(value.object_group);
  return 0;
}

static int set_ObjectInstance_shadow_target(void *self, const PropertyValue &value)
{
  ObjectInstance *obj = reinterpret_cast<ObjectInstance *>(self);
  obj->SetShadowTarget(value.object_group);
  return 0;
}

static int set_Turbulence_lacunarity(void *self, const PropertyValue &value)
{
  Turbulence *turbulence = reinterpret_cast<Turbulence *>(self);
  turbulence->SetLacunarity(value.vector[0]);
  return 0;
}

static int set_Turbulence_gain(void *self, const PropertyValue &value)
{
  Turbulence *turbulence = reinterpret_cast<Turbulence *>(self);
  turbulence->SetGain(value.vector[0]);
  return 0;
}

static int set_Turbulence_octaves(void *self, const PropertyValue &value)
{
  Turbulence *turbulence = reinterpret_cast<Turbulence *>(self);
  turbulence->SetOctaves((int) value.vector[0]);
  return 0;
}

static int set_Turbulence_amplitude(void *self, const PropertyValue &value)
{
  Turbulence *turbulence = reinterpret_cast<Turbulence *>(self);
  turbulence->SetAmplitude(value.vector[0], value.vector[1], value.vector[2]);
  return 0;
}

static int set_Turbulence_frequency(void *self, const PropertyValue &value)
{
  Turbulence *turbulence = reinterpret_cast<Turbulence *>(self);
  turbulence->SetFrequency(value.vector[0], value.vector[1], value.vector[2]);
  return 0;
}

static int set_Turbulence_offset(void *self, const PropertyValue &value)
{
  Turbulence *turbulence = reinterpret_cast<Turbulence *>(self);
  turbulence->SetOffset(value.vector[0], value.vector[1], value.vector[2]);
  return 0;
}

static int set_Renderer_sample_jitter(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetSampleJitter(value.vector[0]);
  return 0;
}

static int set_Renderer_cast_shadow(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetShadowEnable((int) value.vector[0]);
  return 0;
}

static int set_Renderer_max_diffuse_depth(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetMaxDiffuseDepth((int) value.vector[0]);
  return 0;
}

static int set_Renderer_max_reflect_depth(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetMaxReflectDepth((int) value.vector[0]);
  return 0;
}

static int set_Renderer_max_refract_depth(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetMaxRefractDepth((int) value.vector[0]);
  return 0;
}

static int set_Renderer_raymarch_step(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetRaymarchStep(value.vector[0]);
  return 0;
}

static int set_Renderer_raymarch_shadow_step(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetRaymarchShadowStep(value.vector[0]);
  return 0;
}

static int set_Renderer_raymarch_diffuse_step(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetRaymarchDiffuseStep(value.vector[0]);
  return 0;
}

static int set_Renderer_raymarch_reflect_step(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetRaymarchReflectStep(value.vector[0]);
  return 0;
}

static int set_Renderer_raymarch_refract_step(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetRaymarchRefractStep(value.vector[0]);
  return 0;
}

static int set_Renderer_sample_time_range(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetSampleTimeRange(value.vector[0], value.vector[1]);
  return 0;
}

static int set_Renderer_resolution(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetResolution((int) value.vector[0], (int) value.vector[1]);
  return 0;
}

static int set_Renderer_tilesize(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetTileSize((int) value.vector[0], (int) value.vector[1]);
  return 0;
}

static int set_Renderer_filterwidth(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetFilterWidth(value.vector[0], value.vector[1]);
  return 0;
}

static int set_Renderer_sampler_type(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetSamplerType(static_cast<int>(value.vector[0]));
  return 0;
}

static int set_Renderer_pixelsamples(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetPixelSamples((int) value.vector[0], (int) value.vector[1]);
  return 0;
}

static int set_Renderer_adaptive_max_subdivision(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetMaxSubdivision(static_cast<int>(value.vector[0]));
  return 0;
}

static int set_Renderer_adaptive_subdivision_threshold(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetSubdivisionThreshold(value.vector[0]);
  return 0;
}

static int set_Renderer_render_region(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetRenderRegion(
      (int) value.vector[0], (int) value.vector[1],
      (int) value.vector[2], (int) value.vector[3]);
  return 0;
}

static int set_Renderer_use_max_thread(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetUseMaxThread((int) value.vector[0]);
  return 0;
}

static int set_Renderer_thread_count(void *self, const PropertyValue &value)
{
  Renderer *renderer = reinterpret_cast<Renderer *>(self);
  renderer->SetThreadCount((int) value.vector[0]);
  return 0;
}

static int set_Camera_fov(void *self, const PropertyValue &value)
{
  Camera *cam = reinterpret_cast<Camera *>(self);
  cam->SetFov(value.vector[0]);
  return 0;
}

static int set_Camera_znear(void *self, const PropertyValue &value)
{
  Camera *cam = reinterpret_cast<Camera *>(self);
  cam->SetNearPlane(value.vector[0]);
  return 0;
}

static int set_Camera_zfar(void *self, const PropertyValue &value)
{
  Camera *cam = reinterpret_cast<Camera *>(self);
  cam->SetFarPlane(value.vector[0]);
  return 0;
}

static int set_Camera_translate(void *self, const PropertyValue &value)
{
  Camera *cam = reinterpret_cast<Camera *>(self);
  cam->SetTranslate(value.vector[0], value.vector[1], value.vector[2], value.time);
  return 0;
}

static int set_Camera_rotate(void *self, const PropertyValue &value)
{
  Camera *cam = reinterpret_cast<Camera *>(self);
  cam->SetRotate(value.vector[0], value.vector[1], value.vector[2], value.time);
  return 0;
}

static int set_Camera_transform_order(void *self, const PropertyValue &value)
{
  Camera *cam = reinterpret_cast<Camera *>(self);
  cam->SetTransformOrder((int) value.vector[0]);
  return 0;
}

static int set_Camera_rotate_order(void *self, const PropertyValue &value)
{
  Camera *cam = reinterpret_cast<Camera *>(self);
  cam->SetRotateOrder((int) value.vector[0]);
  return 0;
}

static int set_Volume_resolution(void *self, const PropertyValue &value)
{
  Volume *volume = reinterpret_cast<Volume *>(self);
  volume->Resize(value.vector[0], value.vector[1], value.vector[2]);
  return 0;
}

static int set_Volume_bounds_min(void *self, const PropertyValue &value)
{
  Volume *volume = (Volume *) self;
  Box bounds;

  bounds = volume->GetBounds();

  bounds.min.x = value.vector[0];
  bounds.min.y = value.vector[1];
  bounds.min.z = value.vector[2];

  volume->SetBounds(bounds);

  return 0;
}

static int set_Volume_bounds_max(void *self, const PropertyValue &value)
{
  Volume *volume = (Volume *) self;
  Box bounds;

  bounds = volume->GetBounds();

  bounds.max.x = value.vector[0];
  bounds.max.y = value.vector[1];
  bounds.max.z = value.vector[2];

  volume->SetBounds(bounds);

  return 0;
}

static int set_Light_intensity(void *self, const PropertyValue &value)
{
  Light *light = reinterpret_cast<Light *>(self);
  light->SetIntensity(value.vector[0]);
  return 0;
}

static int set_Light_color(void *self, const PropertyValue &value)
{
  Light *light = reinterpret_cast<Light *>(self);
  light->SetColor(value.vector[0], value.vector[1], value.vector[2]);
  return 0;
}

static int set_Light_sample_count(void *self, const PropertyValue &value)
{
  Light *light = reinterpret_cast<Light *>(self);
  light->SetSampleCount((int) value.vector[0]);
  return 0;
}

static int set_Light_double_sided(void *self, const PropertyValue &value)
{
  Light *light = reinterpret_cast<Light *>(self);
  light->SetDoubleSided((bool) value.vector[0]);
  return 0;
}

static int set_Light_environment_map(void *self, const PropertyValue &value)
{
  Light *light = reinterpret_cast<Light *>(self);
  light->SetEnvironmentMap(value.texture);
  return 0;
}

static int set_Light_transform_order(void *self, const PropertyValue &value)
{
  // TODO error handling
  if (!XfmIsTransformOrder((int) value.vector[0]))
    return -1;

  Light *light = reinterpret_cast<Light *>(self);
  light->SetTransformOrder((int) value.vector[0]);
  return 0;
}

static int set_Light_rotate_order(void *self, const PropertyValue &value)
{
  // TODO error handling
  if (!XfmIsRotateOrder((int) value.vector[0]))
    return -1;

  Light *light = reinterpret_cast<Light *>(self);
  light->SetRotateOrder((int) value.vector[0]);
  return 0;
}

static int set_Light_translate(void *self, const PropertyValue &value)
{
  Light *light = reinterpret_cast<Light *>(self);
  light->SetTranslate(value.vector[0], value.vector[1], value.vector[2], value.time);
  return 0;
}

static int set_Light_rotate(void *self, const PropertyValue &value)
{
  Light *light = reinterpret_cast<Light *>(self);
  light->SetRotate(value.vector[0], value.vector[1], value.vector[2], value.time);
  return 0;
}

static int set_Light_scale(void *self, const PropertyValue &value)
{
  Light *light = reinterpret_cast<Light *>(self);
  light->SetScale(value.vector[0], value.vector[1], value.vector[2], value.time);
  return 0;
}

#define END_OF_PROPERTY {PROP_NONE, NULL, {0, 0, 0, 0}, NULL}
static const Property ObjectInstance_properties[] = {
  Property("transform_order", PropScalar(ORDER_SRT), set_ObjectInstance_transform_order),
  Property("rotate_order",    PropScalar(ORDER_ZXY), set_ObjectInstance_rotate_order),
  Property("translate",       PropVector3(0, 0, 0),  set_ObjectInstance_translate),
  Property("rotate",          PropVector3(0, 0, 0),  set_ObjectInstance_rotate),
  Property("scale",           PropVector3(1, 1, 1),  set_ObjectInstance_scale),
  Property("reflect_target",  PropObjectGroup(NULL), set_ObjectInstance_reflect_target),
  Property("refract_target",  PropObjectGroup(NULL), set_ObjectInstance_refract_target),
  Property("shadow_target",   PropObjectGroup(NULL), set_ObjectInstance_shadow_target),
  Property()
};

static const Property Turbulence_properties[] = {
  Property("lacunarity", PropScalar(2),        set_Turbulence_lacunarity),
  Property("gain",       PropScalar(.5),       set_Turbulence_gain),
  Property("octaves",    PropScalar(8),        set_Turbulence_octaves),
  Property("amplitude",  PropVector3(1, 1, 1), set_Turbulence_amplitude),
  Property("frequency",  PropVector3(1, 1, 1), set_Turbulence_frequency),
  Property("offset",     PropVector3(0, 0, 0), set_Turbulence_offset),
  Property()
};

static const Property Renderer_properties[] = {
  Property("sample_jitter",         PropScalar(1),     set_Renderer_sample_jitter),
  Property("cast_shadow",           PropScalar(1),     set_Renderer_cast_shadow),
  Property("max_diffuse_depth",     PropScalar(3),     set_Renderer_max_diffuse_depth),
  Property("max_reflect_depth",     PropScalar(3),     set_Renderer_max_reflect_depth),
  Property("max_refract_depth",     PropScalar(3),     set_Renderer_max_refract_depth),
  Property("raymarch_step",         PropScalar(.05),   set_Renderer_raymarch_step),
  Property("raymarch_shadow_step",  PropScalar(.1),    set_Renderer_raymarch_shadow_step),
  Property("raymarch_diffuse_step", PropScalar(.1),    set_Renderer_raymarch_diffuse_step),
  Property("raymarch_reflect_step", PropScalar(.1),    set_Renderer_raymarch_reflect_step),
  Property("raymarch_refract_step", PropScalar(.1),    set_Renderer_raymarch_refract_step),
  Property("sample_time_range",     PropVector2(0, 1), set_Renderer_sample_time_range),
  Property("resolution",            PropVector2(320, 240), set_Renderer_resolution),
  Property("tilesize",              PropVector2(32, 32),   set_Renderer_tilesize),
  Property("filterwidth",           PropVector2(2, 2),     set_Renderer_filterwidth),
  Property("sampler_type",          PropScalar(0),         set_Renderer_sampler_type),
  Property("pixelsamples",          PropVector2(3, 3),     set_Renderer_pixelsamples),
  Property("adaptive_max_subdivision", PropScalar(1), set_Renderer_adaptive_max_subdivision),
  Property("adaptive_subdivision_threshold", PropScalar(.05), set_Renderer_adaptive_subdivision_threshold),
  Property("render_region",         PropVector4(0, 0, 320, 240), set_Renderer_render_region),
  Property("use_max_thread",        PropScalar(1), set_Renderer_use_max_thread),
  Property("thread_count",          PropScalar(8), set_Renderer_thread_count),
  Property()
};

static const Property Camera_properties[] = {
  Property("transform_order", PropScalar(ORDER_SRT), set_Camera_transform_order),
  Property("rotate_order",    PropScalar(ORDER_ZXY), set_Camera_rotate_order),
  Property("translate",       PropVector3(0, 0, 0),  set_Camera_translate),
  Property("rotate",          PropVector3(0, 0, 0),  set_Camera_rotate),
  Property("fov",             PropScalar(30),        set_Camera_fov),
  Property("znear",           PropScalar(.01),       set_Camera_znear),
  Property("zfar",            PropScalar(1000),      set_Camera_zfar),
  Property()
};

static const Property Volume_properties[] = {
  Property("resolution", PropVector3(0, 0, 0), set_Volume_resolution),
  Property("bounds_min", PropVector3(0, 0, 0), set_Volume_bounds_min),
  Property("bounds_max", PropVector3(0, 0, 0), set_Volume_bounds_max),
  Property()
};

static const Property Light_properties[] = {
  Property("transform_order", PropScalar(ORDER_SRT), set_Light_transform_order),
  Property("rotate_order",    PropScalar(ORDER_ZXY), set_Light_rotate_order),
  Property("translate",       PropVector3(0, 0, 0),  set_Light_translate),
  Property("rotate",          PropVector3(0, 0, 0),  set_Light_rotate),
  Property("scale",           PropVector3(1, 1, 1),  set_Light_scale),
  Property("intensity",       PropScalar(1),         set_Light_intensity),
  Property("color",           PropVector3(1, 1, 1),  set_Light_color),
  Property("sample_count",    PropScalar(16),        set_Light_sample_count),
  Property("double_sided",    PropScalar(0),         set_Light_double_sided),
  Property("environment_map", PropTexture(NULL),     set_Light_environment_map),
  Property()
};

class property_desc {
public:
  int entry_type;
  const char *type_name;
  const Property *property_list;
  void *(*get_entry_from_scene)(const Scene *scene, int index);
};

#define DEFINE_GET_ENTRY_FUNC(type) \
void *get_##type(const Scene *scene, int index) { \
  return (void *) scene->Get##type(index); \
}
#define PROPERTY_DESC(type) {Type_##type, #type, type##_properties, get_##type}
DEFINE_GET_ENTRY_FUNC(ObjectInstance)
DEFINE_GET_ENTRY_FUNC(Turbulence)
DEFINE_GET_ENTRY_FUNC(Renderer)
DEFINE_GET_ENTRY_FUNC(Camera)
DEFINE_GET_ENTRY_FUNC(Volume)
DEFINE_GET_ENTRY_FUNC(Light)
static const property_desc property_desc_list[] = {
  PROPERTY_DESC(ObjectInstance),
  PROPERTY_DESC(Turbulence),
  PROPERTY_DESC(Renderer),
  PROPERTY_DESC(Camera),
  PROPERTY_DESC(Volume),
  PROPERTY_DESC(Light),
  {Type_Begin, NULL, NULL, NULL}
};
#undef DEFINE_GET_ENTRY_FUNC
#undef PROPERTY_DESC

static void *get_builtin_type_entry(Scene *scene, const Entry &entry)
{
  const property_desc *desc = NULL;

  for (desc = property_desc_list; desc->type_name != NULL; desc++) {
    if (desc->entry_type == entry.type) {
      return desc->get_entry_from_scene(scene, entry.index);
    }
  }
  return NULL;
}

static const Property *get_builtin_type_property_list(int entry_type)
{
  const property_desc *desc = NULL;

  for (desc = property_desc_list; desc->type_name != NULL; desc++) {
    if (desc->entry_type == entry_type) {
      return desc->property_list;
    }
  }
  return NULL;
}

static int get_builtin_type_by_name(const char *builtin_type_name)
{
  const property_desc *desc = NULL;

  for (desc = property_desc_list; desc->type_name != NULL; desc++) {
    if (strcmp(desc->type_name, builtin_type_name) == 0) {
      return desc->entry_type;
    }
  }
  return Type_Begin;
}

static const Property *get_property_list(const char *type_name)
{
  /* builtin type properties */
  const int entry_type = get_builtin_type_by_name(type_name);
  const Property *builtin_props = get_builtin_type_property_list(entry_type);

  if (builtin_props != NULL) {
    return builtin_props;
  }

  /* plugin type properties */
  {
    const char *plugin_name = type_name;
    Plugin **plugins = get_scene()->GetPluginList();
    Plugin *found = NULL;
    const int N = (int) get_scene()->GetPluginCount();
    int i;

    for (i = 0; i < N; i++) {
      if (strcmp(plugin_name, plugins[i]->GetName()) == 0) {
        found = plugins[i];
        break;
      }
    }

    if (found == NULL) {
      return NULL;
    } else {
      return found->GetPropertyList();
    }
  }
}

static const Property *get_property_list(const Entry &entry)
{
  // builtin type properties
  const Property *builtin_props = get_builtin_type_property_list(entry.type);

  if (builtin_props != NULL) {
    return builtin_props;
  }

  // plugin type properties
  const ID plugin_id = find_plugin_from(encode_id(entry.type, entry.index));
  const Entry plugin_entry = decode_id(plugin_id);
  Plugin *plugin = get_scene()->GetPlugin(plugin_entry.index);

  if (plugin == NULL) {
    return NULL;
  } else {
    return plugin->GetPropertyList();
  }
}
