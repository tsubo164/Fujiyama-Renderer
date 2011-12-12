/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef MESHIO_H
#define MESHIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

struct Mesh;

struct MeshInput {
	FILE *file;
	int version;
	int nverts;
	int nvert_attrs;
	int nfaces;
	int nface_attrs;

	double *P;
	double *N;
	int *indices;

	char **attr_names;
};

struct MeshOutput {
	FILE *file;
	int version;
	int nverts;
	int nvert_attrs;
	int nfaces;
	int nface_attrs;

	double *P;
	double *N;
	float *Cd;
	float *uv;
	int *indices;
};

enum MshErrorNo {
	ERR_MSH_NOERR = 0,
	ERR_MSH_NOMEM,
	ERR_MSH_NOFILE,
	ERR_MSH_NOTMESH,
	ERR_MSH_BADVER,
	ERR_MSH_BADATTRNAME
};

/* error no interfaces */
extern int MshGetErrorNo(void);
extern const char *MshGetErrorMessage(int err);

/* mesh input file interfaces */
extern struct MeshInput *MshOpenInputFile(const char *filename);
extern void MshCloseInputFile(struct MeshInput *in);
extern int MshReadHeader(struct MeshInput *in);
extern int MshReadAttribute(struct MeshInput *in, void *data);

/* mesh output file interfaces */
extern struct MeshOutput *MshOpenOutputFile(const char *filename);
extern void MshCloseOutputFile(struct MeshOutput *out);
extern void MshWriteFile(struct MeshOutput *out);

/* high level interface for loading mesh file */
extern int MshLoadFile(struct Mesh *mesh, const char *filename);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

