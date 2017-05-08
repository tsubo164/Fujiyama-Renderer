// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_procedure.h"
#include "ply2mesh.h"
#include <iostream>

using namespace fj;

class StanfordPlyProcedure : Procedure {
public:
  StanfordPlyProcedure() : mesh(NULL), filepath(""), io_mode("r") {}
  virtual ~StanfordPlyProcedure() {}

public:
  Mesh *mesh;
  std::string filepath;
  std::string io_mode;

private:
  virtual int run() const;
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "StanfordPlyProcedure";

static int set_mesh(void *self, const PropertyValue &value);
static int set_filepath(void *self, const PropertyValue &value);
static int set_io_mode(void *self, const PropertyValue &value);

static const Property MyPropertyList[] = {
  Property("mesh",         PropMesh(NULL),  set_mesh),
  Property("filepath",     PropString(""),  set_filepath),
  Property("io_mode",      PropString("r"), set_io_mode),
  Property()
};

static const MetaInfo MyMetainfo[] = {
  {"help", "Stanford PLY I/O procedure."},
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
  StanfordPlyProcedure *sfply = new StanfordPlyProcedure();

  return sfply;
}

static void MyDeleteFunction(void *self)
{
  StanfordPlyProcedure *sfply = (StanfordPlyProcedure *) self;
  if (sfply == NULL)
    return;
  delete sfply;
}

int StanfordPlyProcedure::run() const
{
  if (mesh == NULL) {
    return -1;
  }
  if (filepath == "" || filepath.empty()) {
    return -1;
  }

  int err = 0;
  if (io_mode == "r") {
    err = ReadPlyFile(filepath.c_str(), *mesh);
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
  StanfordPlyProcedure *sfply = (StanfordPlyProcedure *) self;

  if (value.mesh == NULL)
    return -1;

  sfply->mesh = value.mesh;

  return 0;
}

static int set_filepath(void *self, const PropertyValue &value)
{
  StanfordPlyProcedure *sfply = (StanfordPlyProcedure *) self;

  if (value.string == NULL)
    return -1;

  sfply->filepath = value.string;

  return 0;
}

static int set_io_mode(void *self, const PropertyValue &value)
{
  StanfordPlyProcedure *sfply = (StanfordPlyProcedure *) self;

  sfply->io_mode = value.string;
  if (sfply->io_mode != "r" && sfply->io_mode != "w") {
    sfply->io_mode = "";
  }

  return 0;
}
