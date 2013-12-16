/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_geo_io.h"
#include "fj_memory.h"
#include "fj_vector.h"
#include "fj_array.h"
#include "fj_iff.h"

#include <stdio.h>
#include <string.h>

typedef const char *src_ptr;
typedef char *dst_ptr;

typedef signed char AttributeType;

enum { ATTRIBUTE_NAME_SIZE = 64 };

#define ATTR_TYPE_LIST \
  ATTR(Int8,    int8_t) \
  ATTR(Int16,   int16_t) \
  ATTR(Int32,   int32_t) \
  ATTR(Int64,   int64_t) \
  ATTR(Float,   float) \
  ATTR(Double,  double) \
  ATTR(Vector3, struct Vector)

enum {
  ATTR_None = 0,
  ATTR_Int,
  ATTR_Double,
  ATTR_Vector3
};

struct GeoAttribute {
  char name[64];
  AttributeType type;
  GeoSize data_size;
  GeoSize data_count;

  src_ptr src_data;
  dst_ptr dst_data;

#if 0
  GeoWriteCallback write_callback;
#endif
};
#define INIT_ATTRIBUTE {{'\0'}, ATTR_None, 0, 0, NULL, NULL}

#if 0
static void write_vec3_callback(const void *data, GeoSize element, int index,
    struct AttributeComponent *value)
{
  const struct Vector *vec = (const struct Vector *) data;

  switch (index) {
  case 0:
    value->real = vec[element].x;
    break;
  case 1:
    value->real = vec[element].y;
    break;
  case 2:
    value->real = vec[element].z;
    break;
  default:
    value->real = 0.;
    break;
  }
}
#endif

static struct GeoAttribute output_attribute_data(
    const void *src_data,
    const char *attr_name,
    AttributeType attr_type,
    GeoSize data_size,
    GeoSize data_count)
{
  const size_t len = strlen(attr_name);
  struct GeoAttribute attr = INIT_ATTRIBUTE;

  if (len > ATTRIBUTE_NAME_SIZE - 1) {
    return attr;
  }

  strncpy(attr.name, attr_name, ATTRIBUTE_NAME_SIZE);
  attr.type = attr_type;
  attr.data_size = data_size;
  attr.data_count = data_count;
  attr.src_data = (src_ptr) src_data;

#if 0
  attr.write_callback = write_vec3_callback;
#endif

  return attr;
}

struct GeoFile {
  IffFile *iff;
  char primitive_type[32];

  GeoSize point_count;
  GeoSize primitive_count;

  int32_t point_attribute_count;
  int32_t primitive_attribute_count;

  struct Array *point_attribute;
  struct Array *primitive_attribute;
};

static void close_geo_file(struct GeoFile *geo)
{
  if (geo == NULL) {
    return;
  }

  ArrFree(geo->point_attribute);
  ArrFree(geo->primitive_attribute);

  IffClose(geo->iff);
  FJ_MEM_FREE(geo);
}

static struct GeoFile *open_geo_file(const char *filename, const char *mode)
{
  struct GeoFile *geo = FJ_MEM_ALLOC(struct GeoFile);
  if (geo == NULL) {
    return NULL;
  }

  geo->iff = IffOpen(filename, mode);
  if (geo->iff == NULL) {
    close_geo_file(geo);
    return NULL;
  }

  {
    const int N = sizeof(geo->primitive_type)/sizeof(geo->primitive_type[0]);
    int i;
    for (i = 0; i < N; i++) {
      geo->primitive_type[i] = '\0';
    }
  }

  geo->point_count = 0;
  geo->primitive_count = 0;

  geo->point_attribute_count = 0;
  geo->primitive_attribute_count = 0;

  geo->point_attribute = ArrNew(sizeof(struct GeoAttribute));
  geo->primitive_attribute = ArrNew(sizeof(struct GeoAttribute));

  return geo;
}

/* GeoInputFile */
struct GeoInputFile {
  struct GeoFile *geo;
};

struct GeoInputFile *GeoOpenInputFile(const char *filename)
{
  struct GeoInputFile *file = FJ_MEM_ALLOC(struct GeoInputFile);
  if (file == NULL) {
    return NULL;
  }

  file->geo = open_geo_file(filename, "rb");
  if (file->geo == NULL) {
    GeoCloseInputFile(file);
    return NULL;
  }

  return file;
}

void GeoCloseInputFile(struct GeoInputFile *file)
{
  if (file == NULL) {
    return;
  }
  close_geo_file(file->geo);
  FJ_MEM_FREE(file);
}

/* GeoOutputFile */
struct GeoOutputFile {
  struct GeoFile *geo;
};

static int get_point_attribute_count(const struct GeoOutputFile *file)
{
  return ArrGetElementCount(file->geo->point_attribute);
}
/*
static struct GeoAttribute *get_point_attribute(struct GeoOutputFile *geo, GeoSize index)
{
  return (struct GeoAttribute *) ArrGet(geo->file->point_attribute, index);
}
*/
static int get_primitive_attribute_count(const struct GeoOutputFile *file)
{
  return ArrGetElementCount(file->geo->primitive_attribute);
}
/*
static struct GeoAttribute *get_primitive_attribute(struct GeoOutputFile *geo, GeoSize index)
{
  return (struct GeoAttribute *) ArrGet(geo->primitive_attribute, index);
}
*/

struct GeoOutputFile *GeoOpenOutputFile(const char *filename)
{
  struct GeoOutputFile *file = FJ_MEM_ALLOC(struct GeoOutputFile);
  if (file == NULL) {
    return NULL;
  }

  file->geo = open_geo_file(filename, "wb");
  if (file->geo == NULL) {
    GeoCloseOutputFile(file);
    return NULL;
  }

  return file;
}

void GeoCloseOutputFile(struct GeoOutputFile *file)
{
  if (file == NULL) {
    return;
  }
  close_geo_file(file->geo);
  FJ_MEM_FREE(file);
}

void GeoSetOutputPointCount(struct GeoOutputFile *file, GeoSize point_count)
{
  file->geo->point_count = point_count;
}

void GeoSetOutputPrimitiveCount(struct GeoOutputFile *file, GeoSize primitive_count)
{
  file->geo->primitive_count = primitive_count;
}

void GeoSetOutputPrimitiveType(struct GeoOutputFile *file, const char *primitive_type)
{
  /* TODO fix hard-coded number */
  strncpy(file->geo->primitive_type, primitive_type, 31);
}

static void set_output_attribute(struct Array *attr_list,
    const char *attr_name,
    const void *attr_data,
    AttributeType attr_type,
    GeoSize data_size,
    GeoSize data_count)
{
  struct GeoAttribute new_attr;
  const int attr_count = ArrGetElementCount(attr_list);
  int i;

  for (i = 0; i < attr_count; i++) {
    const struct GeoAttribute *attr = (const struct GeoAttribute *) ArrGet(attr_list, i);
    if (strcmp(attr->name, attr_name) == 0) {
      /* TODO error handling: already exists */
      return;
    }
  }

  new_attr = output_attribute_data(
      attr_data,
      attr_name,
      attr_type,
      data_size,
      data_count);

  ArrPush(attr_list, &new_attr);
}

#define DEFINE_SET_OUTPUT_POINT_ATTRIBUTE(suffix,type)\
void GeoSetOutputPointAttribute##suffix(struct GeoOutputFile *file, \
    const char *attr_name, const type *attr_data) \
{ \
  set_output_attribute(file->geo->point_attribute, \
  attr_name, attr_data, ATTR_##suffix, sizeof(*(attr_data)), file->geo->point_count); \
}
DEFINE_SET_OUTPUT_POINT_ATTRIBUTE(Double, double)
DEFINE_SET_OUTPUT_POINT_ATTRIBUTE(Vector3, struct Vector)

#if 0
void GeoSetOutputAttribute(struct GeoOutputFile *file,
    const char *attr_name,
    const void *attr_data,
    int element_type, /* point/primitive */
    int data_type,
    int element_count,
    int component_count,
    GeoWriteCallback callback)
{
}
#endif

static void write_attribute_info(IffFile *iff, const struct GeoAttribute *attr)
{
  IffChunk chunk;

  IffWriteChunkGroupBegin(iff, "INFO", &chunk);
  {
    const char *type_name = NULL;
    int8_t nelems = 0;

    switch (attr->type) {
    case ATTR_Double:
      type_name = "Double";
      nelems = 1;
      break;
    case ATTR_Vector3:
      type_name = "Double";
      nelems = 3;
      break;
    default:
      break;
    }

    IffWriteChunkString(iff, "NAME", attr->name);
    IffWriteChunkString(iff, "TYPE", type_name);
    IffWriteChunkInt8(iff, "NELEMS", &nelems, 1);
  }
  IffWriteChunkGroupEnd(iff, &chunk);
}

static void write_attribute_info_list(IffFile *iff,
    const struct GeoAttribute *attr_list, int count)
{
  IffChunk chunk;
  int i;

  IffWriteChunkGroupBegin(iff, "PTALIST", &chunk);
  {
    for (i = 0; i < count; i++) {
      write_attribute_info(iff, &attr_list[i]);
    }
  }
  IffWriteChunkGroupEnd(iff, &chunk);
}

static void write_attribute_data(IffFile *iff, const struct GeoAttribute *attr)
{
  IffChunk chunk;
  IffChunk achk;

  IffWriteChunkGroupBegin(iff, "ATTRDATA", &chunk);
  {
    const char *type_name = NULL;
    int8_t nelems = 0;
    int i;

    switch (attr->type) {
    case ATTR_Double:
      type_name = "Double";
      nelems = 1;
      IffWriteChunkString(iff, "NAME", attr->name);
      IffWriteChunkString(iff, "TYPE", type_name);
      /*
      IffWriteChunkInt8(iff, "NELEMS", &nelems, 1);
      */
      FJ_IFF_WRITE_CHUNK(iff, "NELEMS", &nelems, 1);
      printf("attr->data_size:  %ld\n", attr->data_size);
      printf("attr->data_count: %ld\n", attr->data_count);
      printf(">>>>>>>> %ld\n", attr->data_count);
#if n
      IffWriteChunkDouble(iff, "DATA", (double *) attr->src_data, attr->data_count);
#endif
      IffWriteChunk(iff, "DATA", attr->src_data, attr->data_size, attr->data_count);
      break;

    case ATTR_Vector3:
      type_name = "Double";
      nelems = 3;
      IffWriteChunkString(iff, "NAME", attr->name);
      IffWriteChunkString(iff, "TYPE", type_name);
      IffWriteChunkInt8(iff, "NELEMS", &nelems, 1);

      IffWriteChunkGroupBegin(iff, "DATA", &achk);
#if 0
        for (i = 0; i < attr->data_count; i++) {
          int j;
          for (j = 0; j < 3; j++) {
            struct AttributeComponent value = {0, 0};
            attr->write_callback(attr->src_data, i, j, &value);
            printf("%g ", value.real);
            IffWrite(iff, &value.real, sizeof(double), 1);
          }
          printf("\n");
        }
#endif
        for (i = 0; i < attr->data_count; i++) {
          const struct Vector *v = (const struct Vector *) attr->src_data;
          double d[3];
          d[0] = v[i].x;
          d[1] = v[i].y;
          d[2] = v[i].z;
          IffWriteDouble(iff, d, 3);
        }
      IffWriteChunkGroupEnd(iff, &achk);
      break;

    default:
      break;
    }

  }
  IffWriteChunkGroupEnd(iff, &chunk);
}

void GeoWriteFile(struct GeoOutputFile *file)
{
  const char signature[8] = {(char)128, 'F', 'J', 'G', 'E', 'O', '.', '.'};
  struct GeoFile *geo = file->geo;
  IffFile *iff = geo->iff;

  const char *primitive_type = geo->primitive_type;
  const int64_t point_count = geo->point_count;
  const int64_t primitive_count = geo->primitive_count;
  const int32_t point_attribute_count = get_point_attribute_count(file);
  const int32_t primitive_attribute_count = get_primitive_attribute_count(file);

  IffChunk file_chunk;
  IffChunk geo_chunk;
  IffChunk header_chunk;
  /*
  IffChunk data_chunk;
  */

  IffWriteChunkGroupBegin(iff, signature, &file_chunk);
  {
    IffWriteChunkGroupBegin(iff, primitive_type, &geo_chunk);
    {
      IffWriteChunkGroupBegin(iff, "HEADER", &header_chunk);
      {
        IffWriteChunkInt64(iff, "NPT", &point_count, 1);
        IffWriteChunkInt64(iff, "NPR", &primitive_count, 1);
        IffWriteChunkInt32(iff, "NPTATTR", &point_attribute_count, 1);
        IffWriteChunkInt32(iff, "NPRATTR", &primitive_attribute_count, 1);

        write_attribute_info_list(iff,
            (const struct GeoAttribute *) geo->point_attribute->data,
            point_attribute_count);

        write_attribute_info_list(iff,
            (const struct GeoAttribute *) geo->primitive_attribute->data,
            primitive_attribute_count);
      }
      IffWriteChunkGroupEnd(iff, &header_chunk);

      {
        int i;
        for (i = 0; i < point_attribute_count; i++) {
          const struct GeoAttribute *attr =
              (const struct GeoAttribute *) ArrGet(geo->point_attribute, i);

          write_attribute_data(iff, attr);
#if 0
          IffWriteChunkGroupBegin(iff, "PTADATA", &data_chunk);
          {
            IffWriteChunkString(iff, "PTANAME", attr->name);
            /*
            IffChunk chk;
            IffWriteChunkGroupBegin(iff, "DATA", &chk);
            {
              int j;
              for (j = 0; j < geo->point_count; j++) {
                double d[3];
                const struct Vector *v = (const struct Vector *) attr->src_data;
                d[0] = v[j].x;
                d[1] = v[j].y;
                d[2] = v[j].z;
                IffWriteDouble(iff, d, 3);
                VecPrint(&v[j]);
              }
            }
            IffWriteChunkGroupEnd(iff, &chk);
            */
          }
          IffWriteChunkGroupEnd(iff, &data_chunk);
#endif
        }
      }
    }
    IffWriteChunkGroupEnd(iff, &geo_chunk);
  }
  IffWriteChunkGroupEnd(iff, &file_chunk);
}

static int read_pointcloud_header(struct GeoFile *geo)
{
  IffFile *iff = geo->iff;
  IffChunk geo_chunk;
  IffChunk header_chunk;
  IffChunk attribute_chunk;

  IffReadChunkGroupBegin(iff, "PTCLOUD", &geo_chunk);
  {
    IffReadNextChunk(iff, &header_chunk);
    if (IffChunkMatch(&header_chunk, "HEADER") == 0) {
      return -1;
    }
    IffPutBackChunk(iff, &header_chunk);

    /* TODO fix this: works with even "PTCLOUD"! */
    IffReadChunkGroupBegin(iff, "HEADER", &header_chunk);
    {
      while (IffReadNextChunk(iff, &attribute_chunk)) {

        if (IffChunkMatch(&attribute_chunk, "NPT")) {
          IffReadInt64(iff, &geo->point_count, 1);
        }

        else if (IffChunkMatch(&attribute_chunk, "NPR")) {
          IffReadInt64(iff, &geo->primitive_count, 1);
        }

        else if (IffChunkMatch(&attribute_chunk, "NPTATTR")) {
          IffReadInt32(iff, &geo->point_attribute_count, 1);
        }

        else if (IffChunkMatch(&attribute_chunk, "NPRATTR")) {
          IffReadInt32(iff, &geo->primitive_attribute_count, 1);
        }

        else {
        }

      }
    }
    IffReadChunkGroupEnd(iff, &header_chunk);
  }
  IffReadChunkGroupEnd(iff, &geo_chunk);

  return 0;
}

int GeoReadHeader(struct GeoInputFile *file)
{
  const char signature[8] = {(char)128, 'F', 'J', 'G', 'E', 'O', '.', '.'};
  IffFile *iff = file->geo->iff;

  IffChunk file_chunk;
  IffChunk geo_chunk;

  IffReadNextChunk(iff, &file_chunk);
  if (IffChunkMatch(&file_chunk, signature) == 0) {
    return -1;
  }

  IffPutBackChunk(iff, &file_chunk);
  IffReadChunkGroupBegin(iff, signature, &file_chunk);
  {
    while (IffReadNextChunk(iff, &geo_chunk)) {

      if (IffChunkMatch(&geo_chunk, "PTCLOUD")) {
        IffPutBackChunk(iff, &geo_chunk);
        read_pointcloud_header(file->geo);
        break;
      }

    }
  }
  IffReadChunkGroupEnd(iff, &file_chunk);

  return 0;
}

GeoSize GeoGetInputPointCount(const struct GeoInputFile *file)
{
  return file->geo->point_count;
}

GeoSize GeoGetInputPrimitiveCount(const struct GeoInputFile *file)
{
  return file->geo->primitive_count;
}
