/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef OBJBUFFER_H
#define OBJBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

struct ObjBuffer;
struct Mesh;

extern struct ObjBuffer *ObjBufferNew(void);
extern void ObjBufferFree(struct ObjBuffer *buffer);

extern int ObjBufferFromFile(struct ObjBuffer *buffer, const char *filename);
extern int ObjBufferToMesh(const struct ObjBuffer *buffer, struct Mesh *mesh);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

