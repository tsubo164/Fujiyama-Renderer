/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef __cplusplus
extern "C" {
#endif

struct Texture;

extern struct Texture *TexNew(void);
extern void TexFree(struct Texture *tex);

extern void TexLookup(struct Texture *tex, float u, float v, float *color);
extern int TexLoadFile(struct Texture *tex, const char *filename);

extern void TexGetResolution(const struct Texture *tex, int *xres, int *yres);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

