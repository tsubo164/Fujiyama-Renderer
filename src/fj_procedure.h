// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_PROCEDURE_H
#define FJ_PROCEDURE_H

#include "fj_compatibility.h"
#include "fj_property.h"
#include "fj_plugin.h"
#include <string>

#define PROCEDURE_PLUGIN_TYPE "Procedure"

namespace fj {

class FJ_API ProcedureFunctionTable {
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

class FJ_API Procedure {
public:
  Procedure();
  virtual ~Procedure();

  int Run() const;
  int SetProperty(const std::string &prop_name, const PropertyValue &src_data);

private:
  virtual int run() const = 0;
  virtual const Property *get_property_list() const = 0;
};

} // namespace xxx

#endif // FJ_XXX_H
