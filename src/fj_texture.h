/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_TEXTURE_H
#define FJ_TEXTURE_H

#ifdef __cplusplus
extern "C" {
#endif

struct Texture;
struct Color4;

extern struct Texture *TexNew(void);
extern void TexFree(struct Texture *tex);

extern void TexLookup(struct Texture *tex, float u, float v, struct Color4 *rgba);
extern int TexLoadFile(struct Texture *tex, const char *filename);

extern void TexGetResolution(const struct Texture *tex, int *xres, int *yres);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
