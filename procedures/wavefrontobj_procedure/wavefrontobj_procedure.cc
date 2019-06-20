// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_procedure.h"
#include "fj_triangle.h"
#include "ObjBuffer.h"
#include <iostream>

using namespace fj;

class WavefrontObjProcedure : Procedure {
public:
  WavefrontObjProcedure() : mesh(NULL), filepath(""), io_mode("r") {}
  virtual ~WavefrontObjProcedure() {}

public:
  Mesh *mesh;
  std::string filepath;
  std::string io_mode;

private:
  virtual int run() const;
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "WavefrontObjProcedure";

static int set_mesh(void *self, const PropertyValue &value);
static int set_filepath(void *self, const PropertyValue &value);
static int set_io_mode(void *self, const PropertyValue &value);

static int read_file(const std::string &filepath, Mesh &mesh);

static const Property MyPropertyList[] = {
  Property("mesh",         PropMesh(NULL),  set_mesh),
  Property("filepath",     PropString(""),  set_filepath),
  Property("io_mode",      PropString("r"), set_io_mode),
  Property()
};

static const MetaInfo MyMetainfo[] = {
  {"help", "Wavefront Obj I/O procedure."},
  {"plugin_type", "Procedure"},
  {NULL, NULL}
};

extern "C" {
FJ_PLUGIN_API int Initialize(PluginInfo *info)
{
  return PlgSetupInfo(info,
      PLUGIN_API_VERSION,
      PROCEDURE_PLUGIN_TYPE,
      MyPluginName,
      MyCreateFunction,
      MyDeleteFunction,
      MyPropertyList,
      MyMetainfo);
}
} // extern "C"

static void *MyCreateFunction(void)
{
  WavefrontObjProcedure *wfobj = new WavefrontObjProcedure();

  return wfobj;
}

static void MyDeleteFunction(void *self)
{
  WavefrontObjProcedure *wfobj = (WavefrontObjProcedure *) self;
  if (wfobj == NULL)
    return;
  delete wfobj;
}

int WavefrontObjProcedure::run() const
{
  if (mesh == NULL) {
    return -1;
  }
  if (filepath == "" || filepath.empty()) {
    return -1;
  }

  int err = 0;
  if (io_mode == "r") {
    err = read_file(filepath, *mesh);
  }
  /* not supported yet
  else if (io_mode == "w") {
  } */
  else {
    err = -1;
  }

  return err;
}

static int set_mesh(void *self, const PropertyValue &value)
{
  WavefrontObjProcedure *wfobj = (WavefrontObjProcedure *) self;

  if (value.mesh == NULL)
    return -1;

  wfobj->mesh = value.mesh;

  return 0;
}

static int set_filepath(void *self, const PropertyValue &value)
{
  WavefrontObjProcedure *wfobj = (WavefrontObjProcedure *) self;

  if (value.string == NULL)
    return -1;

  wfobj->filepath = value.string;

  return 0;
}

static int set_io_mode(void *self, const PropertyValue &value)
{
  WavefrontObjProcedure *wfobj = (WavefrontObjProcedure *) self;

  wfobj->io_mode = value.string;
  if (wfobj->io_mode != "r" && wfobj->io_mode != "w") {
    wfobj->io_mode = "";
  }

  return 0;
}

static int read_file(const std::string &filepath, Mesh &mesh)
{
  std::ifstream ifs(filepath.c_str());
  if (!ifs) {
    std::cerr << "error: couldn't open input file: " << filepath << "\n";
    return -1;
  }

  ObjBuffer buffer;
  int err = buffer.Parse(ifs);
  if (err) {
    // TODO error handling
    return -1;
  }

  mesh.Clear();

  err = ObjBufferComputeNormals(buffer);
  if (err) {
    // TODO error handling
    return -1;
  }

  err = ObjBufferToMesh(buffer, mesh);
  if (err) {
    // TODO error handling
    return -1;
  }

  std::cout << "vertex_count: " << buffer.vertex_count << "\n";
  std::cout << "face_count:   " << buffer.face_count << "\n";
  std::cout << "face group count: " << buffer.group_name_to_id.size() << "\n";
  for (std::map<std::string, int>::const_iterator it = buffer.group_name_to_id.begin();
    it != buffer.group_name_to_id.end(); ++it) {
    std::cout << "  name: [" << it->first << "] -> ID: " << it->second << "\n";
  }
  std::cout << "\n";

  return 0;
}
