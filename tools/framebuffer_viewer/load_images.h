/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef LOAD_IMAGES_H
#define LOAD_IMAGES_H

#ifdef __cplusplus
extern "C" {
#endif

struct FrameBuffer;

struct BufferInfo {
  int databox[4];
  int viewbox[4];
  int tilesize;
};
#define BUFINFO_INIT {{0,0,0,0},{0,0,0,0},0}

extern const char *file_extension(const char *filename);

extern int load_fb(const char *filename, struct FrameBuffer *fb, struct BufferInfo *info);

extern int load_mip(const char *filename, struct FrameBuffer *fb, struct BufferInfo *info);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */
