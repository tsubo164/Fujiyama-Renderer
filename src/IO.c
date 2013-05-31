/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "IO.h"
#include "Memory.h"
#include "Vector.h"
#include "Array.h"

#include <stdio.h>
#include <string.h>

typedef const char *src_ptr;
typedef char *dst_ptr;

typedef unsigned char ElmType;
typedef unsigned char NameSize;

enum { MAGIC_SIZE = 4 };
enum { ELEMENT_NAME_SIZE = 64 };

struct ChunkData {
	char element_name[ELEMENT_NAME_SIZE];
	ElmType element_type;
	int element_count;

	src_ptr src_data;
	dst_ptr dst_data;
};
#define INIT_CHUNK {{'\0'}, ELM_NONE, 0, NULL, NULL}

static struct ChunkData input_chunk_data(
		void *dst_data,
		const char *element_name,
		ElmType element_type,
		int element_count)
{
	const size_t len = strlen(element_name);
	struct ChunkData chunk = INIT_CHUNK;

	if (len > ELEMENT_NAME_SIZE - 1) {
		return chunk;
	}

	strncpy(chunk.element_name, element_name, ELEMENT_NAME_SIZE);
	chunk.element_count = element_count;
	chunk.element_type = element_type;
	chunk.dst_data = (dst_ptr) dst_data;

	return chunk;
}

static struct ChunkData output_chunk_data(
		const void *src_data,
		const char *element_name,
		ElmType element_type,
		int element_count)
{
	const size_t len = strlen(element_name);
	struct ChunkData chunk = INIT_CHUNK;

	if (len > ELEMENT_NAME_SIZE - 1) {
		return chunk;
	}

	strncpy(chunk.element_name, element_name, ELEMENT_NAME_SIZE);
	chunk.element_count = element_count;
	chunk.element_type = element_type;
	chunk.src_data = (src_ptr) src_data;

	return chunk;
}

/* ChunkData */
const char *ChkGetElementName(const struct ChunkData *chunk)
{
	return chunk->element_name;
}

int ChkGetElementType(const struct ChunkData *chunk)
{
	return chunk->element_type;
}

int ChkGetElementCount(const struct ChunkData *chunk)
{
	return chunk->element_count;
}

static int copy_magic_number(char *dst, const char *src);
static int read_magic_number(struct InputFile *in);
static int write_magic_number(struct OutputFile *out);

static int write_chunk_info(FILE *file, const struct ChunkData *chunk);
static int write_chunk_data(FILE *file, const struct ChunkData *chunk);

static int read_chunk_info(FILE *file, struct ChunkData *chunk);
static int read_chunk_data(FILE *file, struct ChunkData *chunk);

/* InputFile */
struct InputFile {
	FILE *file;
	char magic[MAGIC_SIZE];

	struct Array *header_chunks;
	struct Array *data_chunks;
	int is_header_ended;
};

struct InputFile *IOOpenInputFile(const char *filename, const char *magic)
{
	int err = 0;
	struct InputFile *in = MEM_ALLOC(struct InputFile);

	if (in == NULL) {
		return NULL;
	}

	in->file = fopen(filename, "r");
	if (in->file == NULL) {
		MEM_FREE(in);
		return NULL;
	}

	in->header_chunks = ArrNew(sizeof(struct ChunkData));
	in->data_chunks = ArrNew(sizeof(struct ChunkData));
	in->is_header_ended = 0;

	err = read_magic_number(in);
	if (err) {
		IOCloseInputFile(in);
		return NULL;
	}
	if (memcmp(in->magic, magic, MAGIC_SIZE) != 0) {
		IOCloseInputFile(in);
		return NULL;
	}

	return in;
}

void IOCloseInputFile(struct InputFile *in)
{
	if (in == NULL) {
		return;
	}
	
	if (in->file != NULL) {
		fclose(in->file);
	}

	ArrFree(in->header_chunks);
	ArrFree(in->data_chunks);
	MEM_FREE(in);
}

int IOGetInputHeaderChunkCount(const struct InputFile *in)
{
	return in->header_chunks->nelems;
}

int IOGetInputDataChunkCount(const struct InputFile *in)
{
	return in->data_chunks->nelems;
}

const struct ChunkData *IOGetInputHeaderChunk(const struct InputFile *in, int index)
{
	if (index < 0 || index > IOGetInputHeaderChunkCount(in)) {
		return NULL;
	}
	return (const struct ChunkData *) ArrGet(in->header_chunks, index);
}

const struct ChunkData *IOGetInputDataChunk(const struct InputFile *in, int index)
{
	if (index < 0 || index > IOGetInputDataChunkCount(in)) {
		return NULL;
	}
	return (const struct ChunkData *) ArrGet(in->data_chunks, index);
}

void IOEndInputHeader(struct InputFile *in)
{
	in->is_header_ended = 1;
}

int IOReadInputHeader(struct InputFile *in)
{
	int header_chunk_count = 0;
	int data_chunk_count = 0;
	size_t nreads = 0;
	int i;

	for (i = 0; i < 12; i++) {
		char c = '\0';
		nreads = fread(&c, sizeof(c), 1, in->file);
	}

	nreads = fread(&header_chunk_count, sizeof(header_chunk_count), 1, in->file);
	nreads = fread(&data_chunk_count,   sizeof(data_chunk_count),   1, in->file);

	for (i = 0; i < in->header_chunks->nelems; i++) {
		struct ChunkData *chunk = (struct ChunkData *) ArrGet(in->header_chunks, i);
		read_chunk_data(in->file, chunk);
	}

	for (i = 0; i < data_chunk_count; i++) {
		struct ChunkData chunk = INIT_CHUNK;
		read_chunk_info(in->file, &chunk);
		ArrPush(in->data_chunks, &chunk);
	}

	return 0;
}

int IOReadInputData(struct InputFile *in)
{
	int n_chunks = 0;
	int i;

	if (in == NULL) {
		return -1;
	}

	if (in->file == NULL) {
		return -1;
	}

	n_chunks = in->data_chunks->nelems;

	for (i = 0; i < n_chunks; i++) {
		struct ChunkData *chunk = (struct ChunkData *) ArrGet(in->data_chunks, i);
		read_chunk_data(in->file, chunk);
	}

	return 0;
}

/* OutputFile */
struct OutputFile {
	FILE *file;
	char magic[MAGIC_SIZE];

	struct Array *header_chunks;
	struct Array *data_chunks;
	int is_header_ended;
};

struct OutputFile *IOOpenOutputFile(const char *filename, const char *magic)
{
	struct OutputFile *out = MEM_ALLOC(struct OutputFile);

	if (out == NULL) {
		return NULL;
	}

	out->file = fopen(filename, "wb");
	if (out->file == NULL) {
		MEM_FREE(out);
		return NULL;
	}

	out->header_chunks = ArrNew(sizeof(struct ChunkData));
	out->data_chunks = ArrNew(sizeof(struct ChunkData));
	out->is_header_ended = 0;

	if (copy_magic_number(out->magic, magic)) {
		IOCloseOutputFile(out);
		return NULL;
	}

	return out;
}

void IOCloseOutputFile(struct OutputFile *out)
{
	if (out == NULL) {
		return;
	}

	if (out->file != NULL) {
		fclose(out->file);
	}

	ArrFree(out->header_chunks);
	ArrFree(out->data_chunks);
	MEM_FREE(out);
}

void IOEndOutputHeader(struct OutputFile *out)
{
	out->is_header_ended = 1;
}

int IOWriteOutputHeader(struct OutputFile *out)
{
	int element_count = 0;
	int err = 0;
	int i;

	if (out == NULL) {
		return -1;
	}

	if (out->file == NULL) {
		return -1;
	}

	err = write_magic_number(out);
	if (err) {
		return -1;
	}

	for (i = 0; i < 12; i++) {
		const char c = '*';
		fwrite(&c, sizeof(c), 1, out->file);
	}

	element_count = out->header_chunks->nelems;
	fwrite(&element_count, sizeof(element_count), 1, out->file);

	element_count = out->data_chunks->nelems;
	fwrite(&element_count, sizeof(element_count), 1, out->file);

	for (i = 0; i < out->header_chunks->nelems; i++) {
		const struct ChunkData *chunk =
				(const struct ChunkData *) ArrGet(out->header_chunks, i);
		write_chunk_data(out->file, chunk);
	}

	for (i = 0; i < out->data_chunks->nelems; i++) {
		const struct ChunkData *chunk =
				(const struct ChunkData *) ArrGet(out->data_chunks, i);
		write_chunk_info(out->file, chunk);
	}

	return 0;
}

int IOWriteOutputData(struct OutputFile *out)
{
	int i;

	if (out == NULL) {
		return -1;
	}

	if (out->file == NULL) {
		return -1;
	}

	for (i = 0; i < out->data_chunks->nelems; i++) {
		const struct ChunkData *chunk =
				(const struct ChunkData *) ArrGet(out->data_chunks, i);
		write_chunk_data(out->file, chunk);
	}

	return 0;
}

void IOSetInputInt(struct InputFile *in,
		const char *element_name,
		int *dst_data,
		int element_count)
{
	if (in->is_header_ended) {
		int i;
		for (i = 0; i < in->data_chunks->nelems; i++) {
			struct ChunkData *chk = (struct ChunkData *) ArrGet(in->data_chunks, i);
			if (strcmp(chk->element_name, element_name) == 0) {
				chk->dst_data = (dst_ptr) dst_data;
				break;
			}
		}
	} else {
		const struct ChunkData chunk = input_chunk_data(
				dst_data,
				element_name,
				ELM_INT,
				element_count);
		ArrPush(in->header_chunks, &chunk);
	}
}

void IOSetInputDouble(struct InputFile *in,
		const char *element_name,
		double *dst_data,
		int element_count)
{
	if (in->is_header_ended) {
		int i;
		for (i = 0; i < in->data_chunks->nelems; i++) {
			struct ChunkData *chk = (struct ChunkData *) ArrGet(in->data_chunks, i);
			if (strcmp(chk->element_name, element_name) == 0) {
				chk->dst_data = (dst_ptr) dst_data;
				return;
			}
		}
	} else {
		const struct ChunkData chunk = input_chunk_data(
				dst_data,
				element_name,
				ELM_DOUBLE,
				element_count);
		ArrPush(in->header_chunks, &chunk);
	}
}

void IOSetInputVector3(struct InputFile *in,
		const char *element_name,
		struct Vector *dst_data,
		int element_count)
{
	if (in->is_header_ended) {
		int i;
		for (i = 0; i < in->data_chunks->nelems; i++) {
			struct ChunkData *chk = (struct ChunkData *) ArrGet(in->data_chunks, i);
			if (strcmp(chk->element_name, element_name) == 0) {
				chk->dst_data = (dst_ptr) dst_data;
				break;
			}
		}
	} else {
		const struct ChunkData chunk = input_chunk_data(
				dst_data,
				element_name,
				ELM_VECTOR3,
				element_count);
		ArrPush(in->header_chunks, &chunk);
	}
}

void IOSetOutputInt(struct OutputFile *out,
		const char *element_name,
		const int *src_data,
		int element_count)
{
	const struct ChunkData chunk = output_chunk_data(
			src_data,
			element_name,
			ELM_INT,
			element_count);

	if (out->is_header_ended) {
		ArrPush(out->data_chunks, &chunk);
	} else {
		ArrPush(out->header_chunks, &chunk);
	}
}

void IOSetOutputDouble(struct OutputFile *out,
		const char *element_name,
		const double *src_data,
		int element_count)
{
	const struct ChunkData chunk = output_chunk_data(
			src_data,
			element_name,
			ELM_DOUBLE,
			element_count);

	if (out->is_header_ended) {
		ArrPush(out->data_chunks, &chunk);
	} else {
		ArrPush(out->header_chunks, &chunk);
	}
}

void IOSetOutputVector3(struct OutputFile *out,
		const char *element_name,
		const struct Vector *src_data,
		int element_count)
{
	const struct ChunkData chunk = output_chunk_data(
			src_data,
			element_name,
			ELM_VECTOR3,
			element_count);

	if (out->is_header_ended) {
		ArrPush(out->data_chunks, &chunk);
	} else {
		ArrPush(out->header_chunks, &chunk);
	}
}

static int copy_magic_number(char *dst, const char *src)
{
	const size_t len = strlen(src);

	if (len != MAGIC_SIZE) {
		return -1;
	}

	strncpy(dst, src, MAGIC_SIZE);
	return 0;
}

static int read_magic_number(struct InputFile *in)
{
	const size_t nreads = fread(in->magic, sizeof(char), MAGIC_SIZE, in->file);

	if (nreads != MAGIC_SIZE) {
		return -1;
	}

	return 0;
}

static int write_magic_number(struct OutputFile *out)
{
	const size_t nwrotes = fwrite(out->magic, sizeof(char), MAGIC_SIZE, out->file);

	if (nwrotes != MAGIC_SIZE) {
		return -1;
	}

	return 0;
}

static int write_chunk_info(FILE *file, const struct ChunkData *chunk)
{
	const NameSize namesize = (NameSize) strlen(chunk->element_name) + 1;

	fwrite(&namesize, sizeof(namesize), 1, file);
	fwrite(chunk->element_name, sizeof(char), namesize, file);
	fwrite(&chunk->element_type, sizeof(chunk->element_type), 1, file);
	fwrite(&chunk->element_count, sizeof(chunk->element_count), 1, file);

	return 0;
}

static int write_chunk_data(FILE *file, const struct ChunkData *chunk)
{
	const int NELEMS = chunk->element_count;
	int i;

	switch (chunk->element_type) {

	case ELM_INT: {
		const int *src_data = (const int *) chunk->src_data;
		for (i = 0; i < NELEMS; i++) {
			fwrite(&src_data[i], sizeof(*src_data), 1, file);
		}
		}
		break;

	case ELM_DOUBLE: {
		const double *src_data = (const double *) chunk->src_data;
		for (i = 0; i < NELEMS; i++) {
			fwrite(&src_data[i], sizeof(*src_data), 1, file);
		}
		}
		break;

	case ELM_VECTOR3: {
		const struct Vector *src_data = (const struct Vector *) chunk->src_data;
		for (i = 0; i < NELEMS; i++) {
			fwrite(&src_data[i].x, sizeof(*src_data), 1, file);
			fwrite(&src_data[i].y, sizeof(*src_data), 1, file);
			fwrite(&src_data[i].z, sizeof(*src_data), 1, file);
		}
		}
		break;

	default:
		break;
	}

	return 0;
}

static int read_chunk_info(FILE *file, struct ChunkData *chunk)
{
	size_t nread = 0;
	NameSize namesize = 0;

	nread = fread(&namesize, sizeof(namesize), 1, file);
	nread = fread(chunk->element_name, sizeof(char), namesize, file);
	nread = fread(&chunk->element_type, sizeof(chunk->element_type), 1, file);
	nread = fread(&chunk->element_count, sizeof(chunk->element_count), 1, file);

	return 0;
}

static int read_chunk_data(FILE *file, struct ChunkData *chunk)
{
	const int NELEMS = chunk->element_count;
	int i;

	if (chunk->dst_data == NULL) {
		return -1;
	}

	switch (chunk->element_type) {

	case ELM_INT: {
		size_t nreads = 0;
		int *dst_data = (int *) chunk->dst_data;
		for (i = 0; i < NELEMS; i++) {
			nreads = fread(&dst_data[i], sizeof(*dst_data), 1, file);
		}
		}
		break;

	case ELM_DOUBLE: {
		size_t nreads = 0;
		double *dst_data = (double *) chunk->dst_data;
		for (i = 0; i < NELEMS; i++) {
			nreads = fread(&dst_data[i], sizeof(*dst_data), 1, file);
		}
		}
		break;

	case ELM_VECTOR3: {
		size_t nreads = 0;
		struct Vector *dst_data = (struct Vector *) chunk->dst_data;
		for (i = 0; i < NELEMS; i++) {
			nreads = fread(&dst_data[i].x, sizeof(*dst_data), 1, file);
			nreads = fread(&dst_data[i].y, sizeof(*dst_data), 1, file);
			nreads = fread(&dst_data[i].z, sizeof(*dst_data), 1, file);
		}
		}
		break;

	default:
		break;
	}

	return 0;
}

