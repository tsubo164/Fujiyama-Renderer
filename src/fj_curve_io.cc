// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_curve_io.h"
#include "fj_curve.h"

#include <cstring>
#include <cstdlib>

#define CRV_FILE_VERSION 1
#define CRV_FILE_MAGIC "CURV"
#define CRV_MAGIC_SIZE 4
#define MAX_ATTRNAME_SIZE 32

namespace fj {

static size_t write_attriname(struct CurveOutput *out, const std::string &name);
static size_t write_attridata(struct CurveOutput *out, const std::string &name);
static void set_error(int err);

static int error_no = ERR_CRV_NOERR;

// error no interfaces
int CrvGetErrorNo(void)
{
  return error_no;
}

const char *CrvGetErrorMessage(int err)
{
  static const char *errmsg[] = {
    "",                         // ERR_CRV_NOERR
    "No memory for CurveInput", // ERR_CRV_NOMEM
    "No such file",             // ERR_CRV_NOFILE
    "Not curve file",           // ERR_CRV_NOTMESH
    "Invalid file version",     // ERR_CRV_BADVER
    "Invalid attribute name"    // ERR_CRV_BADATTRNAME
  };
  static const int nerrs = (int) sizeof(errmsg)/sizeof(errmsg[0]);

  if (err >= nerrs) {
    fprintf(stderr, "fatal error: error no %d is out of range\n", err);
    abort();
  }
  return errmsg[err];
}

static void set_error(int err)
{
  error_no = err;
}

// curve input file interfaces
struct CurveInput *CrvOpenInputFile(const char *filename)
{
  CurveInput *in = new CurveInput();
  if (in == NULL) {
    set_error(ERR_CRV_NOMEM);
    return NULL;
  }

  in->file = fopen(filename, "rb");
  if (in->file == NULL) {
    set_error(ERR_CRV_NOFILE);
    delete in;
    return NULL;
  }

  in->version = 0;
  in->nverts = 0;
  in->nvert_attrs = 0;
  in->ncurves = 0;
  in->ncurve_attrs = 0;
  in->P = NULL;
  in->width = NULL;
  in->Cd = NULL;
  in->uv = NULL;
  in->velocity = NULL;
  in->indices = NULL;

  return in;
}

void CrvCloseInputFile(struct CurveInput *in)
{
  if (in == NULL)
    return;

  if (in->file != NULL) {
    fclose(in->file);
  }
  delete in;
}

int CrvReadHeader(struct CurveInput *in)
{
  int i;
  size_t nreads = 0;
  size_t namesize = 1;
  char magic[CRV_MAGIC_SIZE];
  int nattrs_alloc;

  nreads += fread(magic, sizeof(char), CRV_MAGIC_SIZE, in->file);
  if (memcmp(magic, CRV_FILE_MAGIC, CRV_MAGIC_SIZE) != 0) {
    set_error(ERR_CRV_NOTMESH);
    return -1;
  }
  nreads += fread(&in->version, sizeof(int), 1, in->file);
  if (in->version != CRV_FILE_VERSION) {
    set_error(ERR_CRV_BADVER);
    return -1;
  }
  nreads += fread(&in->nverts, sizeof(int), 1, in->file);
  nreads += fread(&in->nvert_attrs, sizeof(int), 1, in->file);
  nreads += fread(&in->ncurves, sizeof(int), 1, in->file);
  nreads += fread(&in->ncurve_attrs, sizeof(int), 1, in->file);

  nattrs_alloc = in->nvert_attrs + in->ncurve_attrs;
  in->attr_names.resize(nattrs_alloc, "");

  for (i = 0; i < in->nvert_attrs + in->ncurve_attrs; i++) {
    char attrname[MAX_ATTRNAME_SIZE] = {'\0'};

    nreads += fread(&namesize, sizeof(size_t), 1, in->file);
    if (namesize > MAX_ATTRNAME_SIZE-1) {
      set_error(ERR_CRV_BADATTRNAME);
      return -1;
    }
    nreads += fread(attrname, sizeof(char), namesize, in->file);
    in->attr_names[i] = attrname;
  }

  return 0;
}

int CrvReadAttribute(struct CurveInput *in)
{
  size_t nreads = 0;
  size_t datasize = 0;
  nreads += fread(&datasize, sizeof(size_t), 1, in->file);
  in->data_buffer.resize(datasize);
  nreads += fread(&in->data_buffer[0], sizeof(char), datasize, in->file);

  return 0;
}

// curve output file interfaces
struct CurveOutput *CrvOpenOutputFile(const char *filename)
{
  CurveOutput *out = new CurveOutput();
  if (out == NULL) {
    set_error(ERR_CRV_NOMEM);
    return NULL;
  }

  out->file = fopen(filename, "wb");
  if (out->file == NULL) {
    set_error(ERR_CRV_NOFILE);
    delete out;
    return NULL;
  }

  out->version = CRV_FILE_VERSION;
  out->nverts = 0;
  out->nvert_attrs = 0;
  out->ncurves = 0;
  out->ncurve_attrs = 0;
  out->P = NULL;
  out->width = NULL;
  out->Cd = NULL;
  out->uv = NULL;
  out->velocity = NULL;
  out->indices = NULL;

  return out;
}

void CrvCloseOutputFile(struct CurveOutput *out)
{
  if (out == NULL)
    return;

  if (out->file != NULL) {
    fclose(out->file);
  }
  delete out;
}

void CrvWriteFile(struct CurveOutput *out)
{
  char magic[] = CRV_FILE_MAGIC;

  // counts nvert_attrs automatically
  out->nvert_attrs = 0;
  if (out->P != NULL) out->nvert_attrs++;
  if (out->width != NULL) out->nvert_attrs++;
  if (out->Cd != NULL) out->nvert_attrs++;
  if (out->uv != NULL) out->nvert_attrs++;
  if (out->velocity != NULL) out->nvert_attrs++;
  out->ncurve_attrs = 0;
  if (out->indices != NULL) out->ncurve_attrs++;

  fwrite(magic, sizeof(char), CRV_MAGIC_SIZE, out->file);
  fwrite(&out->version, sizeof(int), 1, out->file);
  fwrite(&out->nverts, sizeof(int), 1, out->file);
  fwrite(&out->nvert_attrs, sizeof(int), 1, out->file);
  fwrite(&out->ncurves, sizeof(int), 1, out->file);
  fwrite(&out->ncurve_attrs, sizeof(int), 1, out->file);

  write_attriname(out, "P");
  write_attriname(out, "width");
  write_attriname(out, "Cd");
  write_attriname(out, "uv");
  write_attriname(out, "velocity");
  write_attriname(out, "indices");

  write_attridata(out, "P");
  write_attridata(out, "width");
  write_attridata(out, "Cd");
  write_attridata(out, "uv");
  write_attridata(out, "velocity");
  write_attridata(out, "indices");
}

int CrvLoadFile(struct Curve *curve, const char *filename)
{
  int i;
  int TOTAL_ATTR_COUNT;
  struct CurveInput *in;

  in = CrvOpenInputFile(filename);
  if (in == NULL) {
    return -1;
  }

  if (CrvReadHeader(in)) {
    CrvCloseInputFile(in);
    return -1;
  }

  TOTAL_ATTR_COUNT = in->nvert_attrs + in->ncurve_attrs;

  for (i = 0; i < TOTAL_ATTR_COUNT; i++) {
    const std::string &attrname = in->attr_names[i];
    if (attrname == "P") {
      curve->SetVertexCount(in->nverts);
      curve->AddVertexPosition();
      CrvReadAttribute(in);
      for (int j = 0; j < in->nverts; j++) {
        const Real *data = (const Real *) &in->data_buffer[0];
        const Vector P(
            data[3*j + 0],
            data[3*j + 1],
            data[3*j + 2]);
        curve->SetVertexPosition(j, P);
      }
    }
    else if (attrname == "width") {
      curve->SetVertexCount(in->nverts);
      curve->AddVertexWidth();
      CrvReadAttribute(in);
      for (int j = 0; j < in->nverts; j++) {
        const Real *data = (const Real *) &in->data_buffer[0];
        const Real width = data[j];
        curve->SetVertexWidth(j, width);
      }
    }
    else if (attrname == "Cd") {
      curve->SetVertexCount(in->nverts);
      curve->AddVertexColor();
      CrvReadAttribute(in);
      for (int j = 0; j < in->nverts; j++) {
        const float *data = (const float *) &in->data_buffer[0];
        const Color Cd(
            data[3*j + 0],
            data[3*j + 1],
            data[3*j + 2]);
        curve->SetVertexColor(j, Cd);
      }
    }
    else if (attrname == "uv") {
      curve->SetVertexCount(in->nverts);
      curve->AddVertexTexture();
      CrvReadAttribute(in);
      for (int j = 0; j < in->nverts; j++) {
        const float *data = (const float *) &in->data_buffer[0];
        const TexCoord texcoord(
            data[2*j + 0],
            data[2*j + 1]);
        curve->SetVertexTexture(j, texcoord);
      }
    }
    else if (attrname == "velocity") {
      curve->SetVertexCount(in->nverts);
      curve->AddVertexVelocity();
      CrvReadAttribute(in);
      for (int j = 0; j < in->nverts; j++) {
        const Real *data = (const Real *) &in->data_buffer[0];
        const Vector velocity(
            data[3*j + 0],
            data[3*j + 1],
            data[3*j + 2]);
        curve->SetVertexVelocity(j, velocity);
      }
    }
    else if (attrname == "indices") {
      curve->SetCurveCount(in->ncurves);
      curve->AddCurveIndices();
      CrvReadAttribute(in);
      for (int j = 0; j < in->ncurves; j++) {
        const Index *data = (const Index *) &in->data_buffer[0];
        const Index idx = data[j];
        curve->SetCurveIndices(j, idx);
      }
    }
  }

  curve->ComputeBounds();
  CrvCloseInputFile(in);

  return 0;
}

static size_t write_attriname(struct CurveOutput *out, const std::string &name)
{
  size_t namesize;
  size_t nwrotes;

  if (name == "P" && out->P == NULL) {
    return 0;
  }
  else if (name == "width" && out->width == NULL) {
    return 0;
  }
  else if (name == "Cd" && out->Cd == NULL) {
    return 0;
  }
  else if (name == "uv" && out->uv == NULL) {
    return 0;
  }
  else if (name == "velocity" && out->velocity == NULL) {
    return 0;
  }
  else if (name == "indices" && out->indices == NULL) {
    return 0;
  }

  nwrotes = 0;
  namesize = name.length() + 1;
  nwrotes += fwrite(&namesize, sizeof(size_t), 1, out->file);
  nwrotes += fwrite(name.c_str(), sizeof(char), namesize, out->file);

  return nwrotes;
}

static size_t write_attridata(struct CurveOutput *out, const std::string &name)
{
  size_t datasize;
  size_t nwrotes;

  nwrotes = 0;
  if (name == "P") {
    if (out->P == NULL)
      return 0;
    datasize = 3 * sizeof(double) * out->nverts;
    nwrotes += fwrite(&datasize, sizeof(size_t), 1, out->file);
    nwrotes += fwrite(out->P, sizeof(double), 3 * out->nverts, out->file);
  }
  else if (name == "width") {
    if (out->width == NULL)
      return 0;
    datasize = 1 * sizeof(double) * out->nverts;
    fwrite(&datasize, sizeof(size_t), 1, out->file);
    fwrite(out->width, sizeof(double), 1 * out->nverts, out->file);
  }
  else if (name == "Cd") {
    if (out->Cd == NULL)
      return 0;
    datasize = 3 * sizeof(float) * out->nverts;
    fwrite(&datasize, sizeof(size_t), 1, out->file);
    fwrite(out->Cd, sizeof(float), 3 * out->nverts, out->file);
  }
  else if (name == "uv") {
    if (out->uv == NULL)
      return 0;
    datasize = 2 * sizeof(float) * out->nverts;
    fwrite(&datasize, sizeof(size_t), 1, out->file);
    fwrite(out->uv, sizeof(float), 2 * out->nverts, out->file);
  }
  else if (name == "velocity") {
    if (out->velocity == NULL)
      return 0;
    datasize = 3 * sizeof(double) * out->nverts;
    fwrite(&datasize, sizeof(size_t), 1, out->file);
    fwrite(out->velocity, sizeof(double), 3 * out->nverts, out->file);
  }
  else if (name == "indices") {
    if (out->indices == NULL)
      return 0;
    datasize = 1 * sizeof(int) * out->ncurves;
    fwrite(&datasize, sizeof(size_t), 1, out->file);
    fwrite(out->indices, sizeof(int), 1 * out->ncurves, out->file);
  }
  return nwrotes;
}

} // namespace xxx
