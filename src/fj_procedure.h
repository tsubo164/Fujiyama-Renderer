/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_PROCEDURE_H
#define FJ_PROCEDURE_H

#include "fj_property.h"
#include "fj_plugin.h"

#define PROCEDURE_PLUGIN_TYPE "Procedure"

namespace fj {

struct Procedure;

struct ProcedureFunctionTable {
  int (*MyRun)(void *self);
};

enum PrcErrorNo {
  PRC_ERR_NOERR = 0,
  PRC_ERR_TYPE_NOT_MATCH,
  PRC_ERR_NOOBJ,
  PRC_ERR_NOVTBL,
  PRC_ERR_NOMEM
};

extern struct Procedure *PrcNew(const struct Plugin *plugin);
extern void PrcFree(struct Procedure *procedure);

extern int PrcRun(struct Procedure *procedure);

extern const struct Property *PrcGetPropertyList(const struct Procedure *procedure);
extern int PrcSetProperty(struct Procedure *procedure,
    const char *prop_name, const struct PropertyValue *src_data);

} // namespace xxx

#endif /* FJ_XXX_H */
