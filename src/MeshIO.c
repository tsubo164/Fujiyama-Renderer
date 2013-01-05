/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "MeshIO.h"
#include "Mesh.h"
#include "String.h"
#include <stdlib.h>
#include <string.h>

#define MSH_FILE_VERSION 1
#define MSH_FILE_MAGIC "MESH"
#define MSH_MAGIC_SIZE 4
#define MAX_ATTRNAME_SIZE 32

static int error_no = MSH_ERR_NONE;
static size_t write_attriname(struct MeshOutput *out, const char *name);
static size_t write_attridata(struct MeshOutput *out, const char *name);
static void set_error(int err);

/* mesh input file interfaces */
struct MeshInput *MshOpenInputFile(const char *filename)
{
	struct MeshInput *in;

	in = (struct MeshInput *) malloc(sizeof(struct MeshInput));
	if (in == NULL) {
		set_error(MSH_ERR_NO_MEMORY);
		return NULL;
	}

	in->file = fopen(filename, "rb");
	if (in->file == NULL) {
		set_error(MSH_ERR_FILE_NOT_EXIST);
		free(in);
		return NULL;
	}

	in->version = 0;
	in->nverts = 0;
	in->nvert_attrs = 0;
	in->nfaces = 0;
	in->nface_attrs = 0;
	in->P = NULL;
	in->N = NULL;
	in->indices = NULL;

	return in;
}

void MshCloseInputFile(struct MeshInput *in)
{
	char **name;
	if (in == NULL)
		return;

	for (name = in->attr_names; *name != NULL; name++) {
		*name = StrFree(*name);
	}
	free(in->attr_names);

	if (in->file != NULL) {
		fclose(in->file);
	}
	free(in);
}

int MshReadHeader(struct MeshInput *in)
{
	int i;
	size_t nreads = 0;
	size_t namesize = 1;
	char magic[MSH_MAGIC_SIZE];
	int nattrs_alloc;

	nreads += fread(magic, sizeof(char), MSH_MAGIC_SIZE, in->file);
	if (memcmp(magic, MSH_FILE_MAGIC, MSH_MAGIC_SIZE) != 0) {
		set_error(MSH_ERR_BAD_MAGIC_NUMBER);
		return -1;
	}
	nreads += fread(&in->version, sizeof(int), 1, in->file);
	if (in->version != MSH_FILE_VERSION) {
		set_error(MSH_ERR_BAD_FILE_VERSION);
		return -1;
	}
	nreads += fread(&in->nverts, sizeof(int), 1, in->file);
	nreads += fread(&in->nvert_attrs, sizeof(int), 1, in->file);
	nreads += fread(&in->nfaces, sizeof(int), 1, in->file);
	nreads += fread(&in->nface_attrs, sizeof(int), 1, in->file);

	nattrs_alloc = in->nvert_attrs + in->nface_attrs + 1; /* for sentinel */
	in->attr_names = (char **) malloc(sizeof(char *) * nattrs_alloc);
	for (i = 0; i < nattrs_alloc; i++) {
		in->attr_names[i] = NULL;
	}

	for (i = 0; i < in->nvert_attrs + in->nface_attrs; i++) {
		char attrname[MAX_ATTRNAME_SIZE] = {'\0'};

		nreads += fread(&namesize, sizeof(size_t), 1, in->file);
		if (namesize > MAX_ATTRNAME_SIZE-1) {
			set_error(MSH_ERR_LONG_ATTRIB_NAME);
			return -1;
		}
		nreads += fread(attrname, sizeof(char), namesize, in->file);
		in->attr_names[i] = StrDup(attrname);
	}

	return 0;
}

int MshReadAttribute(struct MeshInput *in, void *data)
{
	size_t nreads = 0;
	size_t datasize = 0;
	nreads += fread(&datasize, sizeof(size_t), 1, in->file);
	nreads += fread(data, sizeof(char), datasize, in->file);

	return 0;
}

/* mesh output file interfaces */
struct MeshOutput *MshOpenOutputFile(const char *filename)
{
	struct MeshOutput *out;

	out = (struct MeshOutput *) malloc(sizeof(struct MeshOutput));
	if (out == NULL) {
		set_error(MSH_ERR_NO_MEMORY);
		return NULL;
	}

	out->file = fopen(filename, "wb");
	if (out->file == NULL) {
		set_error(MSH_ERR_FILE_NOT_EXIST);
		free(out);
		return NULL;
	}

	out->version = MSH_FILE_VERSION;
	out->nverts = 0;
	out->nvert_attrs = 0;
	out->nfaces = 0;
	out->nface_attrs = 0;
	out->P = NULL;
	out->N = NULL;
	out->indices = NULL;

	return out;
}

void MshCloseOutputFile(struct MeshOutput *out)
{
	if (out == NULL)
		return;

	if (out->file != NULL) {
		fclose(out->file);
	}
	free(out);
}

void MshWriteFile(struct MeshOutput *out)
{
	char magic[] = MSH_FILE_MAGIC;

	/* counts nvert_attrs automatically */
	out->nvert_attrs = 0;
	if (out->P != NULL) out->nvert_attrs++;
	if (out->N != NULL) out->nvert_attrs++;
	if (out->Cd != NULL) out->nvert_attrs++;
	if (out->uv != NULL) out->nvert_attrs++;
	out->nface_attrs = 0;
	if (out->indices != NULL) out->nface_attrs++;

	fwrite(magic, sizeof(char), MSH_MAGIC_SIZE, out->file);
	fwrite(&out->version, sizeof(int), 1, out->file);
	fwrite(&out->nverts, sizeof(int), 1, out->file);
	fwrite(&out->nvert_attrs, sizeof(int), 1, out->file);
	fwrite(&out->nfaces, sizeof(int), 1, out->file);
	fwrite(&out->nface_attrs, sizeof(int), 1, out->file);

	write_attriname(out, "P");
	write_attriname(out, "N");
	write_attriname(out, "Cd");
	write_attriname(out, "uv");
	write_attriname(out, "indices");

	write_attridata(out, "P");
	write_attridata(out, "N");
	write_attridata(out, "Cd");
	write_attridata(out, "uv");
	write_attridata(out, "indices");
}

int MshLoadFile(struct Mesh *mesh, const char *filename)
{
	int i;
	int TOTAL_ATTR_COUNT;
	struct MeshInput *in;

	in = MshOpenInputFile(filename);
	if (in == NULL) {
		return -1;
	}

	if (MshReadHeader(in)) {
		MshCloseInputFile(in);
		return -1;
	}

	TOTAL_ATTR_COUNT = in->nvert_attrs + in->nface_attrs;

	for (i = 0; i < TOTAL_ATTR_COUNT; i++) {
		const char *attrname;
		attrname = in->attr_names[i];
		if (strcmp(attrname, "P") == 0) {
			MshAllocateVertex(mesh, "P", in->nverts);
			MshReadAttribute(in, mesh->P);
		}
		else if (strcmp(attrname, "N") == 0) {
			MshAllocateVertex(mesh, "N", in->nverts);
			MshReadAttribute(in, mesh->N);
		}
		else if (strcmp(attrname, "uv") == 0) {
			MshAllocateVertex(mesh, "uv", in->nverts);
			MshReadAttribute(in, mesh->uv);
		}
		else if (strcmp(attrname, "indices") == 0) {
			MshAllocateFace(mesh, "indices", in->nfaces);
			MshReadAttribute(in, mesh->indices);
		}
	}

	MshComputeBounds(mesh);
	MshCloseInputFile(in);

	return 0;
}

int MshGetErrorNo(void)
{
	return error_no;
}

static void set_error(int err)
{
	error_no = err;
}

static size_t write_attriname(struct MeshOutput *out, const char *name)
{
	size_t namesize;
	size_t nwrotes;

	if (strcmp(name, "P") == 0 && out->P == NULL) {
		return 0;
	}
	else if (strcmp(name, "N") == 0 && out->N == NULL) {
		return 0;
	}
	else if (strcmp(name, "Cd") == 0 && out->Cd == NULL) {
		return 0;
	}
	else if (strcmp(name, "uv") == 0 && out->uv == NULL) {
		return 0;
	}
	else if (strcmp(name, "indices") == 0 && out->indices == NULL) {
		return 0;
	}

	nwrotes = 0;
	namesize = strlen(name) + 1;
	nwrotes += fwrite(&namesize, sizeof(size_t), 1, out->file);
	nwrotes += fwrite(name, sizeof(char), namesize, out->file);

	return nwrotes;
}

static size_t write_attridata(struct MeshOutput *out, const char *name)
{
	size_t datasize;
	size_t nwrotes;

	nwrotes = 0;
	if (strcmp(name, "P") == 0) {
		if (out->P == NULL)
			return 0;
		datasize = 3 * sizeof(double) * out->nverts;
		nwrotes += fwrite(&datasize, sizeof(size_t), 1, out->file);
		nwrotes += fwrite(out->P, sizeof(double), 3 * out->nverts, out->file);
	}
	else if (strcmp(name, "N") == 0) {
		if (out->N == NULL)
			return 0;
		datasize = 3 * sizeof(double) * out->nverts;
		fwrite(&datasize, sizeof(size_t), 1, out->file);
		fwrite(out->N, sizeof(double), 3 * out->nverts, out->file);
	}
	else if (strcmp(name, "Cd") == 0) {
		if (out->Cd == NULL)
			return 0;
		datasize = 3 * sizeof(float) * out->nverts;
		fwrite(&datasize, sizeof(size_t), 1, out->file);
		fwrite(out->Cd, sizeof(float), 3 * out->nverts, out->file);
	}
	else if (strcmp(name, "uv") == 0) {
		if (out->uv == NULL)
			return 0;
		datasize = 2 * sizeof(float) * out->nverts;
		fwrite(&datasize, sizeof(size_t), 1, out->file);
		fwrite(out->uv, sizeof(float), 2 * out->nverts, out->file);
	}
	else if (strcmp(name, "indices") == 0) {
		if (out->indices == NULL)
			return 0;
		datasize = 3 * sizeof(int) * out->nfaces;
		fwrite(&datasize, sizeof(size_t), 1, out->file);
		fwrite(out->indices, sizeof(int), 3 * out->nfaces, out->file);
	}
	return nwrotes;
}

