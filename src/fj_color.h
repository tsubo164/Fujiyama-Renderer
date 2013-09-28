/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef COLOR_H
#define COLOR_H

#ifdef __cplusplus
extern "C" {
#endif

struct Color {
  double r, g, b;
};

struct Color4 {
  double r, g, b, a;
};

extern struct Color *ColAlloc(long count);
extern struct Color *ColRealloc(struct Color *c, long count);
extern void ColFree(struct Color *c);

#define ColSet(dst,R,G,B) do { \
  (dst)->r = (R); \
  (dst)->g = (G); \
  (dst)->b = (B); \
  } while(0)

#define COL_LERP(dst,A,B,t) do { \
  (dst)->r = (1-(t)) * (A)->r + (t) * (B)->r; \
  (dst)->g = (1-(t)) * (A)->g + (t) * (B)->g; \
  (dst)->b = (1-(t)) * (A)->b + (t) * (B)->b; \
  } while(0)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */
