/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "ObjParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum { INITIAL_INDEX_ALLOC = 8 };

enum TripletType {
	FACE_V = 0,
	FACE_V_VT,
	FACE_V__VN,
	FACE_V_VT_VN
};

struct VectorValue {
	int min_ncomponents;
	int max_ncomponents;
	int scanned_ncomponents;
	double x, y, z, w;
};
#define INIT_VECTOR_VALUE {3, 3, 0, 0., 0., 0., 0.}

struct IndexList {
	int triplet;
	long *vertex;
	long *texture;
	long *normal;
	long count;
	long alloc;
};

static struct IndexList *new_index_list(void);
static void free_index_list(struct IndexList *list);
static void push_index(struct IndexList *list, long v, long vt, long vn);
static void clear_index_list(struct IndexList *list);

static const char *scan_vector(const char *line, struct VectorValue *vector);
static const char *scan_index_list(struct IndexList *list, const char *line);
static int detect_triplet(const char *line);

static char *trim_line(char *line);
static int is_separator(int c);
static int match_token(const char *line, const char *token);

struct ObjParser {
	void *interpreter;
	ReadVertxFunction ReadVertex;
	ReadVertxFunction ReadTexture;
	ReadVertxFunction ReadNormal;
	ReadFaceFunction  ReadFace;
};

struct ObjParser *ObjParserNew(
		void *interpreter,
		ReadVertxFunction read_vertex_function,
		ReadVertxFunction read_texture_function,
		ReadVertxFunction read_normal_function,
		ReadFaceFunction read_face_function)
{
	struct ObjParser *parser = (struct ObjParser *) malloc(sizeof(struct ObjParser));
	if (parser == NULL)
		return NULL;

	parser->interpreter = interpreter;
	parser->ReadVertex = read_vertex_function;
	parser->ReadTexture = read_texture_function;
	parser->ReadNormal = read_normal_function;
	parser->ReadFace = read_face_function;

	return parser;
}

void ObjParserFree(struct ObjParser *parser)
{
	if (parser == NULL)
		return;

	free(parser);
}

int ObjParse(struct ObjParser *parser, const char *filename)
{
	struct IndexList *list = new_index_list();
	FILE *file = fopen(filename, "r");
	char buf[4096] = {'\0'};
	int err = 0;

	if (file == NULL) {
		goto parse_error;
	}

	while (fgets(buf, 4096, file)) {
		const char *line = trim_line(buf);

		if (match_token(line, "v")) {
			struct VectorValue vector = INIT_VECTOR_VALUE;
			vector.min_ncomponents = 3;
			vector.max_ncomponents = 4;
			if (parser->ReadVertex == NULL) {
				continue;
			}

			line += 1; /* skip "v" */
			line = scan_vector(line, &vector);
			if (line == NULL) {
				goto parse_error;
			}

			if (line[0] != '\0') {
				goto parse_error;
			}

			err = parser->ReadVertex(parser->interpreter,
					vector.scanned_ncomponents,
					vector.x, vector.y, vector.z, vector.w);
			if (err) {
				goto parse_error;
			}
		}
		else if (match_token(line, "vt")) {
			struct VectorValue vector = INIT_VECTOR_VALUE;
			vector.min_ncomponents = 1;
			vector.max_ncomponents = 3;
			if (parser->ReadTexture == NULL) {
				continue;
			}

			line += 2; /* skip "vt" */
			line = scan_vector(line, &vector);
			if (line == NULL) {
				goto parse_error;
			}

			if (line[0] != '\0') {
				goto parse_error;
			}

			err = parser->ReadTexture(parser->interpreter,
					vector.scanned_ncomponents,
					vector.x, vector.y, vector.z, vector.w);
			if (err) {
				goto parse_error;
			}
		}
		else if (match_token(line, "vn")) {
			struct VectorValue vector = INIT_VECTOR_VALUE;
			vector.min_ncomponents = 3;
			vector.max_ncomponents = 3;
			if (parser->ReadNormal == NULL) {
				continue;
			}

			line += 2; /* skip "vn" */
			line = scan_vector(line, &vector);
			if (line == NULL) {
				goto parse_error;
			}

			if (line[0] != '\0') {
				goto parse_error;
			}

			err = parser->ReadNormal(parser->interpreter,
					vector.scanned_ncomponents,
					vector.x, vector.y, vector.z, vector.w);
			if (err) {
				goto parse_error;
			}
		}
		else if (match_token(line, "f")) {
			const long *vertex = NULL;
			const long *texture = NULL;
			const long *normal = NULL;
			if (parser->ReadFace == NULL) {
				continue;
			}

			line += 1; /* skip "f" */
			line = scan_index_list(list, line);
			if (line == NULL) {
				goto parse_error;
			}

			switch(list->triplet) {
			case FACE_V:
				vertex  = list->vertex;
				break;
			case FACE_V_VT:
				vertex  = list->vertex;
				texture = list->texture;
				break;
			case FACE_V__VN:
				vertex  = list->vertex;
				break;
			case FACE_V_VT_VN:
				vertex  = list->vertex;
				texture = list->texture;
				normal  = list->normal;
				break;
			default:
				break;
			}

			err = parser->ReadFace(parser->interpreter,
					list->count,
					vertex,
					texture,
					normal);
			if (err) {
				goto parse_error;
			}
		}
	}

	free_index_list(list);
	return 0;

parse_error:
	free_index_list(list);
	return -1;
}

static const char *scan_vector(const char *line, struct VectorValue *vector)
{
	double vec[4] = {0, 0, 0, 0};
	const char *next = line;
	const int min = vector->min_ncomponents < 4 ? vector->min_ncomponents : 4;
	const int max = vector->max_ncomponents < 4 ? vector->max_ncomponents : 4;
	int i;

	for (i = 0; i < min; i++) {
		char *end = NULL;
		vec[i] = strtod(next, &end);
		if (next == end) { return NULL;
		}
		next = end;
	}

	vector->scanned_ncomponents = vector->min_ncomponents;
	for (; i < max; i++) {
		char *end = NULL;
		vec[i] = strtod(next, &end);
		if (next == end) {
			break;
		}
		vector->scanned_ncomponents++;
		next = end;
	}

	vector->x = vec[0];
	vector->y = vec[1];
	vector->z = vec[2];
	vector->w = vec[3];

	return next;
}

static const char *scan_index_list(struct IndexList *list, const char *line)
{
	const char *next = line;
	char *end = NULL;
	long v  = 0;
	long vt = 0;
	long vn = 0;

	clear_index_list(list);
	list->triplet = detect_triplet(line);

	switch (list->triplet) {
	case FACE_V:
		for (;;) {
			v = strtol(next, &end, 0);
			if (next == end) {
				break;
			}

			push_index(list, v, vt, vn);
			next = end;
		}
		break;

	case FACE_V_VT:
		for (;;) {
			v = strtol(next, &end, 0);
			if (next == end) {
				break;
			}

			next = end + 1; /* end[0] == '/' */
			vt = strtol(next, &end, 0);
			if (next == end) {
				break;
			}

			push_index(list, v, vt, vn);
			next = end;
		}
		break;

	case FACE_V__VN:
		for (;;) {
			v = strtol(next, &end, 0);
			if (next == end) {
				break;
			}

			next = end + 2; /* end[0] == "//" */
			vn = strtol(next, &end, 0);
			if (next == end) {
				break;
			}

			push_index(list, v, vt, vn);
			next = end;
		}
		break;

	case FACE_V_VT_VN:
		for (;;) {
			v = strtol(next, &end, 0);
			if (next == end) {
				break;
			}

			next = end + 1; /* end[0] == '/' */
			vt = strtol(next, &end, 0);
			if (next == end) {
				break;
			}

			next = end + 1; /* end[0] == '/' */
			vn = strtol(next, &end, 0);
			if (next == end) {
				break;
			}

			push_index(list, v, vt, vn);
			next = end;
		}
		break;

	default:
		break;
	}

	if (list->count == 0) {
		return NULL;
	}
	return next;
}

static int detect_triplet(const char *line)
{
	const char *ch = line + strspn(line, " \t\v\f");
	int nslashes = 0;

	while (ch[0] != ' ') {
		if (ch[0] == '/') {
			nslashes++;
			if (ch[1] == '/') {
				return FACE_V__VN;
			}
			if (nslashes == 2) {
				return FACE_V_VT_VN;
			}
		}
		ch++;
	}

	if (nslashes == 0)
		return FACE_V;
	else
		return FACE_V_VT;
}

static struct IndexList *new_index_list(void)
{
	struct IndexList *list = (struct IndexList *) malloc(sizeof(struct IndexList));
	if (list == NULL)
		return NULL;

	list->triplet = FACE_V;
	list->count = 0;
	list->alloc = INITIAL_INDEX_ALLOC;
	list->vertex  = (long *) malloc(sizeof(long) * list->alloc);
	list->texture = (long *) malloc(sizeof(long) * list->alloc);
	list->normal  = (long *) malloc(sizeof(long) * list->alloc);

	return list;
}

static void free_index_list(struct IndexList *list)
{
	if (list == NULL)
		return;

	if (list->vertex != NULL)
		free(list->vertex);
	if (list->texture != NULL)
		free(list->texture);
	if (list->normal != NULL)
		free(list->normal);

	free(list);
}

static void push_index(struct IndexList *list, long v, long vt, long vn)
{
	const long next_index = list->count;

	if (list->count == list->alloc) {
		const long new_alloc = list->alloc * 2;
		list->vertex  = (long *) realloc(list->vertex,  sizeof(long) * new_alloc);
		list->texture = (long *) realloc(list->texture, sizeof(long) * new_alloc);
		list->normal  = (long *) realloc(list->normal,  sizeof(long) * new_alloc);
		list->alloc = new_alloc;
	}

	list->vertex[next_index] = v;
	list->texture[next_index] = vt;
	list->normal[next_index] = vn;
	list->count++;
}

static void clear_index_list(struct IndexList *list)
{
	list->triplet = FACE_V;
	list->count = 0;
}

static char *trim_line(char *line)
{
	char *begin = line + strspn(line, " \t\v\f");
	char *last_non_space = begin;
	char *ch = begin;

	while (*ch != '\0') {
		if (!isspace(*ch)) {
			last_non_space = ch;
		}
		ch++;
	}
	last_non_space[1] = '\0';

	return begin;
}

static int is_separator(int c)
{
	if (c == ' ' ||
		c == '\t' ||
		c == '\v' ||
		c == '\f' ||
		c == '\n' ||
		c == '\0')
		return 1;
	else
		return 0;
}

static int match_token(const char *line, const char *token)
{
	const char *ln = line;
	const char *tk = token;

	while (*tk != '\0') {
		if (*tk++ != *ln++) {
			return 0;
		}
	}
	return is_separator(*ln);
}

