/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_geometry.h"
#include "fj_string_function.h"
#include "fj_memory.h"
#include "fj_vector.h"
#include "fj_array.h"
#include "fj_box.h"
#include "fj_iff.h"

#include <string.h>
#include <assert.h>

static const char SIGNATURE[8] = {(char)128, 'F', 'J', 'G', 'E', 'O', '.', '.'};

#define VALUE_TYPE_LIST(TYPE) \
  TYPE(Double,  double) \
  TYPE(Vector3, struct Vector)

#if 0
#define VALUE_TYPE_LIST(TYPE) \
  TYPE(Int8,    int8_t) \
  TYPE(Int16,   int16_t) \
  TYPE(Int32,   int32_t) \
  TYPE(Int64,   int64_t) \
  TYPE(Float,   float) \
  TYPE(Double,  double) \
  TYPE(Vector3, struct Vector)
#endif

enum {
#define VALUE_TYPE_ENUM(suffix,type) ATTR_##suffix,
VALUE_TYPE_LIST(VALUE_TYPE_ENUM)
#undef VALUE_TYPE_ENUM
  ATTR_none
};

static const struct TypeNameMap {
  int type;
  const char *name;
} type_name_map[] = {
#define VALUE_TYPE_MAP(suffix,type) {ATTR_##suffix, #suffix},
VALUE_TYPE_LIST(VALUE_TYPE_MAP)
#undef VALUE_TYPE_MAP
  {ATTR_none, "none"}
};
static const int type_name_count = sizeof(type_name_map)/sizeof(type_name_map[0]);

/* attributes interfaces */
struct Attribute {
  char *name;
  int data_type;
  size_t data_size;
  GeoIndex data_count;
  char *data;
};

struct AttributeList {
  struct Array *data_array;
};

static int get_attribute_count(const struct AttributeList *attr_list)
{
  return ArrGetElementCount(attr_list->data_array);
}

static struct Attribute *get_attribute(const struct AttributeList *attr_list, int index)
{
  struct Attribute **attr_array = (struct Attribute **) ArrGet(attr_list->data_array, 0);
  return attr_array[index];
}

static void push_attribute(struct AttributeList *attr_list, const struct Attribute *attr)
{
  ArrPushPointer(attr_list->data_array, attr);
}

static struct Attribute *find_attribute(const struct AttributeList *attr_list, const char *name)
{
  const int N = get_attribute_count(attr_list);
  int i;
  for (i = 0; i < N; i++) {
    struct Attribute *attr = get_attribute(attr_list, i);
    if (strcmp(attr->name, name) == 0) {
      return attr;
    }
  }
  return NULL;
}

static DataSize get_size_of_data(int data_type)
{
  switch (data_type) {
#define SIZEOF(suffix,type) case ATTR_##suffix: return sizeof(type);
VALUE_TYPE_LIST(SIZEOF)
#undef SIZEOF
  default:
    assert(!"error");
    break;
  }
  return 0;
}

static int string_to_data_type(const char *type_name)
{
  int i;
  for (i = 0; i < type_name_count; i++) {
    if (strcmp(type_name_map[i].name, type_name) == 0) {
      return type_name_map[i].type;
    }
  }
  assert(!"error");
  return 0;
}

#if 0
static const char *data_type_to_string(int data_type)
{
  int i;
  for (i = 0; i < type_name_count; i++) {
    if (type_name_map[i].type == data_type) {
      return type_name_map[i].name;
    }
  }
  assert(!"error");
  return NULL;
}
#endif

struct Attribute *new_attribute(const char *name, int data_type, GeoIndex data_count)
{
  struct Attribute *attr = FJ_MEM_ALLOC(struct Attribute);

  if (attr == NULL) {
    return NULL;
  }

  attr->name = StrDup(name);
  attr->data_type = data_type;
  attr->data_size = get_size_of_data(data_type);
  attr->data_count = data_count;
  attr->data = NULL;

  if (data_count > 0) {
    attr->data = (char *) malloc(attr->data_size * attr->data_count);
  }

  return attr;
}

void free_attribute(struct Attribute *attr)
{
  if (attr == NULL) {
    return;
  }

  if (attr->data != NULL) {
    free(attr->data);
  }

  StrFree(attr->name);
  free(attr);
}

static void free_attribute_list(struct AttributeList *attr_list)
{
  int N = 0;
  int i;

  if (attr_list == NULL) {
    return;
  }

  N = get_attribute_count(attr_list);
  for (i = 0; i < N; i++) {
    free_attribute(get_attribute(attr_list, i));
  }

  if (attr_list->data_array != NULL) {
    ArrFree(attr_list->data_array);
  }

  FJ_MEM_FREE(attr_list);
}

static struct AttributeList *new_attribute_list(void)
{
  struct AttributeList *attr_list = FJ_MEM_ALLOC(struct AttributeList);
  if (attr_list == NULL) {
    return NULL;
  }

  attr_list->data_array = ArrNew(sizeof(struct Attribute *));
  if (attr_list->data_array == NULL) {
    free_attribute_list(attr_list);
    return NULL;
  }

  return attr_list;
}

/* Geometry interfaces */
struct Geometry {
  GeoIndex point_count;
  GeoIndex primitive_count;

  struct Box bounds;

  struct AttributeList *point_attribute;
  struct AttributeList *primitive_attribute;
};

struct Geometry *GeoNew(void)
{
  struct Geometry *geo = FJ_MEM_ALLOC(struct Geometry);

  if (geo == NULL) {
    return NULL;
  }

  GeoSetPointCount(geo, 0);
  GeoSetPrimitiveCount(geo, 0);

  BOX3_SET(&geo->bounds, 0, 0, 0, 0, 0, 0);

  geo->point_attribute = new_attribute_list();
  geo->primitive_attribute = new_attribute_list();

  return geo;
}

void GeoFree(struct Geometry *geo)
{
  if (geo == NULL) {
    return;
  }

  free_attribute_list(geo->point_attribute);
  free_attribute_list(geo->primitive_attribute);

  FJ_MEM_FREE(geo);
}

void GeoSetPointCount(struct Geometry *geo, GeoIndex point_count)
{
  geo->point_count = point_count;
}

void GeoSetPrimitiveCount(struct Geometry *geo, GeoIndex primitive_count)
{
  geo->primitive_count = primitive_count;
}

GeoIndex GeoGetPointCount(const struct Geometry *geo)
{
  return geo->point_count;
}

GeoIndex GeoGetPrimitiveCount(const struct Geometry *geo)
{
  return geo->primitive_count;
}

void *GeoAddAttribute(struct Geometry *geo,
    const char *attr_name, int attr_class, int data_type)
{
  struct AttributeList *attr_list = NULL;
  struct Attribute *found_attr = NULL;
  struct Attribute *new_attr = NULL;
  GeoIndex data_count = 0;

  switch (attr_class) {
  case GEO_POINT:
    attr_list = geo->point_attribute;
    data_count = geo->point_count;
    break;
  case GEO_PRIMITIVE:
    attr_list = geo->primitive_attribute;
    data_count = geo->primitive_count;
    break;
  default:
    assert(!"error");
    break;
  }

  found_attr = find_attribute(attr_list, attr_name);
  if (found_attr != NULL) {
    return found_attr->data;
  }

  new_attr = new_attribute(attr_name, data_type, data_count);
  push_attribute(attr_list, new_attr);

  return new_attr->data;
}

void *GeoGetAttribute(struct Geometry *geo,
    const char *attr_name, int attr_class, int data_type)
{
  struct AttributeList *attr_list = NULL;
  struct Attribute *attr = NULL;

  switch (attr_class) {
  case GEO_POINT:
    attr_list = geo->point_attribute;
    break;
  case GEO_PRIMITIVE:
    attr_list = geo->primitive_attribute;
    break;
  default:
    assert(!"error");
    break;
  }

  attr = find_attribute(attr_list, attr_name);
  if (attr != NULL && attr->data_type == data_type) {
    return attr->data;
  }
  return NULL;
}

#define DEFINE_GET_ATTRIBUTE(suffix,type) \
type *GeoAddAttribute##suffix(struct Geometry *geo, \
    const char *attr_name, int attr_class) \
{ \
  return (type *) GeoAddAttribute(geo, attr_name, attr_class, ATTR_##suffix); \
} \
type *GeoGetAttribute##suffix(struct Geometry *geo, \
    const char *attr_name, int attr_class) \
{ \
  return (type *) GeoGetAttribute(geo, attr_name, attr_class, ATTR_##suffix); \
}
VALUE_TYPE_LIST(DEFINE_GET_ATTRIBUTE)
#undef DEFINE_GET_ATTRIBUTE

static void write_attribute_info(IffFile *iff, const struct Attribute *attr)
{
  IffChunk chunk;

  IffWriteChunkGroupBegin(iff, "ATTR", &chunk);
  {
    const char *type_name = NULL;

    switch (attr->data_type) {
    case ATTR_Double:
      type_name = "Double";
      break;
    case ATTR_Vector3:
      type_name = "Vector3";
      break;
    default:
      assert("!error");
      break;
    }

    IffWriteChunkString(iff, "NAME", attr->name);
    IffWriteChunkString(iff, "TYPE", type_name);
  }
  IffWriteChunkGroupEnd(iff, &chunk);
}

static void write_attribute_info_list(IffFile *iff,
    const char *chunk_id, const struct AttributeList *attr_list)
{
  const int count = get_attribute_count(attr_list);
  IffChunk chunk;
  int i;

  IffWriteChunkGroupBegin(iff, chunk_id, &chunk);
  {
    for (i = 0; i < count; i++) {
      write_attribute_info(iff, get_attribute(attr_list, i));
    }
  }
  IffWriteChunkGroupEnd(iff, &chunk);
}

static void write_attribute_data_Double(IffFile *iff, const struct Attribute *attr)
{
  IffWriteDouble(iff, (const double *) attr->data, attr->data_count);
}
static void write_attribute_data_Vector3(IffFile *iff, const struct Attribute *attr)
{
  const struct Vector *vec = (const struct Vector *) attr->data;
  int i;
  for (i = 0; i < attr->data_count; i++) {
    double values[3] = {0, 0, 0};
    values[0] = vec[i].x;
    values[1] = vec[i].y;
    values[2] = vec[i].z;
    IffWriteDouble(iff, values, 3);
  }
}

static void write_attribute_data(IffFile *iff, const struct Attribute *attr)
{
  IffChunk chunk;
  IffChunk value_chunk;

  IffWriteChunkGroupBegin(iff, "ATTR", &chunk);
  {
    IffWriteChunkString(iff, "NAME", attr->name);

    IffWriteChunkGroupBegin(iff, "DATA", &value_chunk);
    {
      switch (attr->data_type) {
#define WRITE(suffix,type) case ATTR_##suffix: write_attribute_data_##suffix(iff, attr); break;
VALUE_TYPE_LIST(WRITE)
#undef WRITE
      default:
        assert("!error");
        break;
      }
    }
    IffWriteChunkGroupEnd(iff, &value_chunk);
  }
  IffWriteChunkGroupEnd(iff, &chunk);
}

static void write_attribute_data_list(IffFile *iff,
    const char *chunk_id, const struct AttributeList *attr_list)
{
  const int count = get_attribute_count(attr_list);
  IffChunk chunk;
  int i;

  IffWriteChunkGroupBegin(iff, chunk_id, &chunk);
  {
    for (i = 0; i < count; i++) {
      write_attribute_data(iff, get_attribute(attr_list, i));
    }
  }
  IffWriteChunkGroupEnd(iff, &chunk);
}

int GeoWriteFile(const struct Geometry *geo, const char *filename)
{
  IffFile *iff = IffOpen(filename, "wb");

  const char *primitive_type = "PTCLOUD";
  const int64_t point_count = geo->point_count;
  const int64_t primitive_count = geo->primitive_count;
  const int32_t point_attribute_count = get_attribute_count(geo->point_attribute);
  const int32_t primitive_attribute_count = get_attribute_count(geo->primitive_attribute);

  IffChunk file_chunk;
  IffChunk geo_chunk;
  IffChunk header_chunk;
  IffChunk body_chunk;

  IffWriteChunkGroupBegin(iff, SIGNATURE, &file_chunk);
  {
    IffWriteChunkGroupBegin(iff, primitive_type, &geo_chunk);
    {
      IffWriteChunkGroupBegin(iff, "HEADER", &header_chunk);
      {
        IffWriteChunkInt64(iff, "NPT", &point_count, 1);
        IffWriteChunkInt64(iff, "NPR", &primitive_count, 1);
        IffWriteChunkInt32(iff, "NPTATTR", &point_attribute_count, 1);
        IffWriteChunkInt32(iff, "NPRATTR", &primitive_attribute_count, 1);

        write_attribute_info_list(iff, "PTATTR", geo->point_attribute);

        write_attribute_info_list(iff, "PRATTR", geo->primitive_attribute);

      }
      IffWriteChunkGroupEnd(iff, &header_chunk);

      IffWriteChunkGroupBegin(iff, "BODY", &body_chunk);
      {

        write_attribute_data_list(iff, "PTATTR", geo->point_attribute);

        write_attribute_data_list(iff, "PRATTR", geo->primitive_attribute);

      }
      IffWriteChunkGroupEnd(iff, &body_chunk);
    }
    IffWriteChunkGroupEnd(iff, &geo_chunk);
  }
  IffWriteChunkGroupEnd(iff, &file_chunk);

  IffClose(iff);
  return 0;
}

static void read_attribute_info(IffFile *iff,
  struct AttributeList *attr_list, GeoIndex data_count)
{
  IffChunk attr_chunk;
  IffChunk chunk;
  int err = 0;

  err = IffReadChunkGroupBegin(iff, "ATTR", &attr_chunk);
  if (err) {
    /* TODO error handling */
    return;
  }

  {
    char attr_name[64] = {'\0'};
    char type_name[16] = {'\0'};

    while (IffReadNextChildChunk(iff, &attr_chunk, &chunk)) {

      if (IffChunkMatch(&chunk, "NAME")) {
        IffReadString(iff, attr_name);
        printf("NAME: [%s]\n", attr_name);

      } else if (IffChunkMatch(&chunk, "TYPE")) {
        IffReadString(iff, type_name);
        printf("TYPE: [%s]\n", type_name);

      } else {
        IffSkipCurrentChunk(iff, &chunk);
      }

    }

    if (strcmp(attr_name, "") !=0 && strcmp(type_name, "") !=0) {
      const int data_type = string_to_data_type(type_name);
      struct Attribute *found_attr = find_attribute(attr_list, attr_name);
      if (found_attr == NULL) {
        struct Attribute *new_attr = new_attribute(attr_name, data_type, data_count);
        push_attribute(attr_list, new_attr);
      }
    }
  }

  IffReadChunkGroupEnd(iff, &attr_chunk);
}

static void read_attribute_info_list(IffFile *iff,
    const char *chunk_id, struct AttributeList *attr_list, GeoIndex data_count)
{
  int err = 0;
  IffChunk info_list_chunk;
  IffChunk info_chunk;

  err = IffReadChunkGroupBegin(iff, chunk_id, &info_list_chunk);
  if (err) {
    assert(!"error");
  }

  {
    while (IffReadNextChildChunk(iff, &info_list_chunk, &info_chunk)) {

      if (IffChunkMatch(&info_chunk, "ATTR")) {
        IffPutBackChunk(iff, &info_chunk);
        read_attribute_info(iff, attr_list, data_count);

      } else {
        IffSkipCurrentChunk(iff, &info_chunk);
      }
    }
  }

  IffReadChunkGroupEnd(iff, &info_list_chunk);
}

static void read_attribute_data_Double(IffFile *iff, struct Attribute *attr)
{
  IffReadDouble(iff, (double *) attr->data, attr->data_count);
}
static void read_attribute_data_Vector3(IffFile *iff, struct Attribute *attr)
{
  struct Vector *vec = (struct Vector *) attr->data;
  int i;
  for (i = 0; i < attr->data_count; i++) {
    double values[3] = {0, 0, 0};
    IffReadDouble(iff, values, 3);
    vec[i].x = values[0];
    vec[i].y = values[1];
    vec[i].z = values[2];
  }
}

static void read_attribute_data(IffFile *iff, struct AttributeList *attr_list)
{
  IffChunk attr_chunk;
  IffChunk chunk;
  int err = 0;

  err = IffReadChunkGroupBegin(iff, "ATTR", &attr_chunk);
  if (err) {
    /* TODO error handling */
    return;
  }

  {
    char attr_name[64] = {'\0'};

    while (IffReadNextChildChunk(iff, &attr_chunk, &chunk)) {

      if (IffChunkMatch(&chunk, "NAME")) {
        IffReadString(iff, attr_name);

      } else if (IffChunkMatch(&chunk, "DATA")) {
        struct Attribute *attr = find_attribute(attr_list, attr_name);
        if (attr == NULL) {
          /* TODO error handling */
          continue;
        }

        switch (attr->data_type) {
#define READ(suffix,type) case ATTR_##suffix: read_attribute_data_##suffix(iff, attr); break;
VALUE_TYPE_LIST(READ)
#undef READ
        default:
          assert("!error");
          break;
        }

      } else {
        IffSkipCurrentChunk(iff, &chunk);
      }

    }
  }

  IffReadChunkGroupEnd(iff, &attr_chunk);
}

static void read_attribute_data_list(IffFile *iff,
    const char *chunk_id, struct AttributeList *attr_list)
{
  int err = 0;
  IffChunk info_list_chunk;
  IffChunk info_chunk;

  err = IffReadChunkGroupBegin(iff, chunk_id, &info_list_chunk);
  if (err) {
    assert(!"error");
  }

  {
    while (IffReadNextChildChunk(iff, &info_list_chunk, &info_chunk)) {

      if (IffChunkMatch(&info_chunk, "ATTR")) {
        IffPutBackChunk(iff, &info_chunk);
        read_attribute_data(iff, attr_list);

      } else {
        IffSkipCurrentChunk(iff, &info_chunk);
      }
    }
  }

  IffReadChunkGroupEnd(iff, &info_list_chunk);
}

int GeoReadFile(struct Geometry *geo, const char *filename)
{
  IffFile *iff = IffOpen(filename, "rb");

  /*
  const char *primitive_type = "PTCLOUD";
  const int64_t point_count = geo->point_count;
  const int64_t primitive_count = geo->primitive_count;
  const int32_t point_attribute_count = ArrGetElementCount(geo->point_attribute);
  const int32_t primitive_attribute_count = ArrGetElementCount(geo->primitive_attribute);
  */

  IffChunk file_chunk;
  IffChunk geo_chunk;
  IffChunk header_chunk;
  IffChunk body_chunk;
  {
    IffChunk chunk;
    IffReadNextChunk(iff, &chunk);
    if (IffChunkMatch(&chunk, SIGNATURE) == 0) {
      IffClose(iff);
      return -1;
    }
    IffPutBackChunk(iff, &chunk);
  }

  IffReadChunkGroupBegin(iff, SIGNATURE, &file_chunk);
  {
    IffReadChunkGroupBegin(iff, "PTCLOUD", &geo_chunk);
    {
      IffReadChunkGroupBegin(iff, "HEADER", &header_chunk);
      {
        IffChunk chk;
        while (IffReadNextChildChunk(iff, &header_chunk, &chk)) {

          if (IffChunkMatch(&chk, "NPT")) {
            int64_t n = 0;
            IffReadInt64(iff, &n, 1);
            GeoSetPointCount(geo, n);
            printf("NPT: %ld\n", n);

          } else if (IffChunkMatch(&chk, "NPR")) {
            int64_t n = 0;
            IffReadInt64(iff, &n, 1);
            GeoSetPrimitiveCount(geo, n);
            printf("NPR: %ld\n", n);

          } else if (IffChunkMatch(&chk, "PTATTR")) {
            IffPutBackChunk(iff, &chk);
            read_attribute_info_list(iff, "PTATTR", geo->point_attribute,
                GeoGetPointCount(geo));

          } else if (IffChunkMatch(&chk, "PRATTR")) {
            IffPutBackChunk(iff, &chk);
            read_attribute_info_list(iff, "PRATTR", geo->primitive_attribute,
                GeoGetPrimitiveCount(geo));

          } else {
            IffSkipCurrentChunk(iff, &chk);
          }
            IffSkipCurrentChunk(iff, &chk);

        }
      }
      IffReadChunkGroupEnd(iff, &header_chunk);

      IffReadChunkGroupBegin(iff, "BODY", &body_chunk);
      {
        IffChunk chk;
        while (IffReadNextChildChunk(iff, &body_chunk, &chk)) {

          if (IffChunkMatch(&chk, "PTATTR")) {
            IffPutBackChunk(iff, &chk);
            read_attribute_data_list(iff, "PTATTR", geo->point_attribute);

          } else if (IffChunkMatch(&chk, "PRATTR")) {
            IffPutBackChunk(iff, &chk);
            read_attribute_data_list(iff, "PRATTR", geo->primitive_attribute);

          } else {
            IffSkipCurrentChunk(iff, &chk);
          }

        }
      }
      IffReadChunkGroupEnd(iff, &body_chunk);
    }
    IffReadChunkGroupEnd(iff, &geo_chunk);
  }
  IffReadChunkGroupEnd(iff, &file_chunk);

  IffClose(iff);
  return 0;
}
