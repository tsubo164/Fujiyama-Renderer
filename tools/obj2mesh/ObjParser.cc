// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "ObjParser.h"
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

namespace fj {

enum { INITIAL_INDEX_ALLOC = 8 };

enum TripletType {
  FACE_V = 0,
  FACE_V_VT,
  FACE_V__VN,
  FACE_V_VT_VN
};

class VectorValue {
public:
  VectorValue() :
      min_ncomponents(3),
      max_ncomponents(3),
      scanned_ncomponents(0),
      x(0.), y(0.), z(0.), w(0.)
  {}
  ~VectorValue() {}

public:
  int min_ncomponents;
  int max_ncomponents;
  int scanned_ncomponents;
  double x, y, z, w;
};

class IndexList {
public:
  IndexList() :
      triplet(FACE_V),
      vertex (INITIAL_INDEX_ALLOC),
      texture(INITIAL_INDEX_ALLOC),
      normal (INITIAL_INDEX_ALLOC)
  {
  }
  ~IndexList() {}

public:
  int triplet;
  std::vector<long> vertex;
  std::vector<long> texture;
  std::vector<long> normal;
};

static void push_index(IndexList *list, long v, long vt, long vn);
static void clear_index_list(IndexList *list);

static const char *scan_vector(const char *line, VectorValue *vector);
static const char *scan_index_list(IndexList *list, const char *line);
static int detect_triplet(const char *line);

static char *trim_line(char *line);
static int is_separator(int c);
static int match_token(const char *line, const char *token);

class ObjParser {
public:
  ObjParser() {}
  ~ObjParser() {}

public:
  void *interpreter;
  ReadVertxFunction ReadVertex;
  ReadVertxFunction ReadTexture;
  ReadVertxFunction ReadNormal;
  ReadFaceFunction  ReadFace;
};

ObjParser *ObjParserNew(
    void *interpreter,
    ReadVertxFunction read_vertex_function,
    ReadVertxFunction read_texture_function,
    ReadVertxFunction read_normal_function,
    ReadFaceFunction read_face_function)
{
  ObjParser *parser = new ObjParser();

  parser->interpreter = interpreter;
  parser->ReadVertex = read_vertex_function;
  parser->ReadTexture = read_texture_function;
  parser->ReadNormal = read_normal_function;
  parser->ReadFace = read_face_function;

  return parser;
}

void ObjParserFree(ObjParser *parser)
{
  if (parser == NULL)
    return;

  delete parser;
}

int ObjParse(ObjParser *parser, const char *filename)
{
  IndexList list;
  FILE *file = fopen(filename, "r");
  char buf[4096] = {'\0'};
  int err = 0;

  if (file == NULL) {
    goto parse_error;
  }

  while (fgets(buf, 4096, file)) {
    const char *line = trim_line(buf);

    if (match_token(line, "v")) {
      VectorValue vector;
      vector.min_ncomponents = 3;
      vector.max_ncomponents = 4;
      if (parser->ReadVertex == NULL) {
        continue;
      }

      line += 1; // skip "v"
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
      VectorValue vector;
      vector.min_ncomponents = 1;
      vector.max_ncomponents = 3;
      if (parser->ReadTexture == NULL) {
        continue;
      }

      line += 2; // skip "vt"
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
      VectorValue vector;
      vector.min_ncomponents = 3;
      vector.max_ncomponents = 3;
      if (parser->ReadNormal == NULL) {
        continue;
      }

      line += 2; // skip "vn"
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

      line += 1; // skip "f"
      line = scan_index_list(&list, line);
      if (line == NULL) {
        goto parse_error;
      }

      switch(list.triplet) {
      case FACE_V:
        vertex  = &list.vertex[0];
        break;
      case FACE_V_VT:
        vertex  = &list.vertex[0];
        texture = &list.texture[0];
        break;
      case FACE_V__VN:
        vertex  = &list.vertex[0];
        break;
      case FACE_V_VT_VN:
        vertex  = &list.vertex[0];
        texture = &list.texture[0];
        normal  = &list.normal[0];
        break;
      default:
        break;
      }

      err = parser->ReadFace(parser->interpreter,
          list.vertex.size(),
          vertex,
          texture,
          normal);
      if (err) {
        goto parse_error;
      }
    }
  }

  fclose(file);
  return 0;

parse_error:
  if (file != NULL) {
    fclose(file);
  }
  return -1;
}

static const char *scan_vector(const char *line, VectorValue *vector)
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

static const char *scan_index_list(IndexList *list, const char *line)
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

      next = end + 1; // end[0] == '/'
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

      next = end + 2; // end[0] == "//"
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

      next = end + 1; // end[0] == '/'
      vt = strtol(next, &end, 0);
      if (next == end) {
        break;
      }

      next = end + 1; // end[0] == '/'
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

  if (list->vertex.empty()) {
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

static void push_index(IndexList *list, long v, long vt, long vn)
{
  list->vertex.push_back(v);
  list->texture.push_back(vt);
  list->normal.push_back(vn);
}

static void clear_index_list(IndexList *list)
{
  list->triplet = FACE_V;
  list->vertex.clear();
  list->texture.clear();
  list->normal.clear();
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

} // namespace xxx
