/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TEXCOORD_H
#define TEXCOORD_H

#ifdef __cplusplus
extern "C" {
#endif

struct TexCoord {
	float u, v;
};

extern struct TexCoord *TexCoordAlloc(long count);
extern struct TexCoord *TexCoordRealloc(struct TexCoord *texcoord, long count);
extern void TexCoordFree(struct TexCoord *texcoord);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

