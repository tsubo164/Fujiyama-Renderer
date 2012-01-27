/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef ARRAY_H
#define ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

struct Array {
	size_t elemsize;
	size_t nelems;
	size_t nallocs;
	char *data;
};

extern struct Array *ArrNew(size_t size_of_element);
extern void ArrFree(struct Array *a);

/* returns the pointer of the head of array */
extern char *ArrPush(struct Array *a, const void *data);

/* returns the pointer of the head of array */
extern char *ArrPushPointer(struct Array *a, const void *pointer);

/* grows array if needed, then returns the head of data. */
extern char *ArrGrow(struct Array *a, size_t new_alloc);

extern char *ArrGet(const struct Array *a, int index);

struct Int1Array { int *data; };
struct Int2Array { int *data; };
struct Int3Array { int *data; };
struct Float1Array { float *data; };
struct Float2Array { float *data; };
struct Float3Array { float *data; };
struct Double1Array { double *data; };
struct Double2Array { double *data; };
struct Double3Array { double *data; };

/* IntArray */
extern struct Int1Array Int1ArrayNew(size_t nelems);
extern struct Int1Array Int1ArrayFree(struct Int1Array array);
extern int *Int1ArrayGetWritable(struct Int1Array array, int i);
extern const int *Int1ArrayGetReadOnly(struct Int1Array array, int i);

extern struct Int2Array Int2ArrayNew(size_t nelems);
extern struct Int2Array Int2ArrayFree(struct Int2Array array);
extern int *Int2ArrayGetWritable(struct Int2Array array, int i);
extern const int *Int2ArrayGetReadOnly(struct Int2Array array, int i);

extern struct Int3Array Int3ArrayNew(size_t nelems);
extern struct Int3Array Int3ArrayFree(struct Int3Array array);
extern int *Int3ArrayGetWritable(struct Int3Array array, int i);
extern const int *Int3ArrayGetReadOnly(struct Int3Array array, int i);

/* FloatArray */
extern struct Float1Array Float1ArrayNew(size_t nelems);
extern struct Float1Array Float1ArrayFree(struct Float1Array array);
extern float *Float1ArrayGetWritable(struct Float1Array array, int i);
extern const float *Float1ArrayGetReadOnly(struct Float1Array array, int i);

extern struct Float2Array Float2ArrayNew(size_t nelems);
extern struct Float2Array Float2ArrayFree(struct Float2Array array);
extern float *Float2ArrayGetWritable(struct Float2Array array, int i);
extern const float *Float2ArrayGetReadOnly(struct Float2Array array, int i);

extern struct Float3Array Float3ArrayNew(size_t nelems);
extern struct Float3Array Float3ArrayFree(struct Float3Array array);
extern float *Float3ArrayGetWritable(struct Float3Array array, int i);
extern const float *Float3ArrayGetReadOnly(struct Float3Array array, int i);

/* DoubleArray */
extern struct Double1Array Double1ArrayNew(size_t nelems);
extern struct Double1Array Double1ArrayFree(struct Double1Array array);
extern double *Double1ArrayGetWritable(struct Double1Array array, int i);
extern const double *Double1ArrayGetReadOnly(struct Double1Array array, int i);

extern struct Double2Array Double2ArrayNew(size_t nelems);
extern struct Double2Array Double2ArrayFree(struct Double2Array array);
extern double *Double2ArrayGetWritable(struct Double2Array array, int i);
extern const double *Double2ArrayGetReadOnly(struct Double2Array array, int i);

extern struct Double3Array Double3ArrayNew(size_t nelems);
extern struct Double3Array Double3ArrayFree(struct Double3Array array);
extern double *Double3ArrayGetWritable(struct Double3Array array, int i);
extern const double *Double3ArrayGetReadOnly(struct Double3Array array, int i);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

