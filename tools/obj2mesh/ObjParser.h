// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef OBJPARSER_H
#define OBJPARSER_H

namespace fj {

struct ObjParser;

typedef int (*ReadVertxFunction)(
    void *interpreter,
    int scanned_ncomponents,
    double x,
    double y,
    double z,
    double w);

typedef int (*ReadFaceFunction)(
    void *interpreter,
    long index_count,
    const long *vertex_indices,
    const long *texture_indices,
    const long *normal_indices);

extern struct ObjParser *ObjParserNew(
    void *interpreter,
    ReadVertxFunction read_vertex_function,
    ReadVertxFunction read_texture_function,
    ReadVertxFunction read_normal_function,
    ReadFaceFunction read_face_function);

extern void ObjParserFree(struct ObjParser *parser);

extern int ObjParse(struct ObjParser *parser, const char *filename);

} // namespace xxx

#endif // FJ_XXX_H
