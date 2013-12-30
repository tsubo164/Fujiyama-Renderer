/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_GEO_IO_H
#define FJ_GEO_IO_H

#include "fj_compatibility.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Vector;

/* TODO make fj_types.h */
typedef int64_t GeoSize;

/* GeoInputFile */
struct GeoInputFile;

extern struct GeoInputFile *GeoOpenInputFile(const char *filename);
extern void GeoCloseInputFile(struct GeoInputFile *file);

extern int GeoReadHeader(struct GeoInputFile *file);

extern GeoSize GeoGetInputPointCount(const struct GeoInputFile *file);
extern GeoSize GeoGetInputPrimitiveCount(const struct GeoInputFile *file);

/* GeoOutputFile */
struct GeoOutputFile;

extern struct GeoOutputFile *GeoOpenOutputFile(const char *filename);
extern void GeoCloseOutputFile(struct GeoOutputFile *file);

extern void GeoSetOutputPointCount(struct GeoOutputFile *file, GeoSize point_count);
extern void GeoSetOutputPrimitiveCount(struct GeoOutputFile *file, GeoSize primitive_count);
extern void GeoSetOutputPrimitiveType(struct GeoOutputFile *file, const char *primitive_type);

extern void GeoSetOutputPointAttributeDouble(struct GeoOutputFile *file,
    const char *attr_name, const double *attr_data);
extern void GeoSetOutputPointAttributeVector3(struct GeoOutputFile *file,
    const char *attr_name, const struct Vector *attr_data);

extern void GeoWriteFile2(struct GeoOutputFile *file);

#if 0
struct AttributeComponent {
  long integer;
  double real;
};

typedef void (*GeoWriteCallback)(const void *data, GeoSize element, int index,
    struct AttributeComponent *value);

enum {
  CLASS_POINT,
  CLASS_PRIMITIVE
};
enum {
  GEO_Double,
  GEO_hoge
};
extern void GeoSetOutputAttribute(struct GeoOutputFile *file,
    const char *attr_name,
    const void *attr_data,
    int element_type, /* point/primitive */
    int data_type,
    int element_count,
    int component_count,
    GeoWriteCallback callback);


/* TODO */
struct GeoAttribute;
typedef void (*GeoWriteCallback)(struct GeoOutputFile *file,
    const struct GeoAttribute *attr, GeoSize index);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
