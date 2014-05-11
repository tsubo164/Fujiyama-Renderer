/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_GEOMETRY_H
#define FJ_GEOMETRY_H

#include "fj_compatibility.h"
#include <stdio.h>

namespace fj {

struct Geometry;
typedef int64_t GeoIndex;

enum GeoAttributeClass {
  GEO_POINT = 0,
  GEO_PRIMITIVE
};

extern struct Geometry *GeoNew(void);
extern void GeoFree(struct Geometry *geo);

extern void GeoSetPointCount(struct Geometry *geo, GeoIndex point_count);
extern void GeoSetPrimitiveCount(struct Geometry *geo, GeoIndex primitive_count);

extern GeoIndex GeoGetPointCount(const struct Geometry *geo);
extern GeoIndex GeoGetPrimitiveCount(const struct Geometry *geo);

/*
 * Writes geometry to a file. geo_type should be like MESH, CURVES, PTCLOUD, etc ...
 * This geometry module doesn't know about the rule of the types.
 */
extern int GeoWriteFile(const struct Geometry *geo, const char *filename, const char *geo_type);
extern int GeoReadFile(struct Geometry *geo, const char *filename);

extern double *GeoAddAttributeDouble(struct Geometry *geo,
    const char *attr_name, int attr_class);
extern struct Vector *GeoAddAttributeVector3(struct Geometry *geo,
    const char *attr_name, int attr_class);

extern double *GeoGetAttributeDouble(struct Geometry *geo,
    const char *attr_name, int attr_class);
extern struct Vector *GeoGetAttributeVector3(struct Geometry *geo,
    const char *attr_name, int attr_class);

} // namespace xxx

#endif /* FJ_XXX_H */
