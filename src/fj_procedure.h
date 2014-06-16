// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_PROCEDURE_H
#define FJ_PROCEDURE_H

#include "fj_property.h"
#include "fj_plugin.h"
#include <string>

#define PROCEDURE_PLUGIN_TYPE "Procedure"

namespace fj {

class ProcedureFunctionTable {
public:
  int (*MyRun)(void *self);
};

enum PrcErrorNo {
  PRC_ERR_NOERR = 0,
  PRC_ERR_TYPE_NOT_MATCH,
  PRC_ERR_NOOBJ,
  PRC_ERR_NOVTBL,
  PRC_ERR_NOMEM
};

class Procedure {
public:
  Procedure();
  ~Procedure();

  int Initialize(const Plugin *plugin);
  int Run();

  const Property *GetPropertyList() const;
  int SetProperty(const std::string &prop_name, const PropertyValue &src_data);

public:
  void *self_;
  const ProcedureFunctionTable *vptr_;
  const Plugin *plugin_;
};

extern Procedure *PrcNew(const Plugin *plugin);
extern void PrcFree(Procedure *procedure);

} // namespace xxx

#endif // FJ_XXX_H
