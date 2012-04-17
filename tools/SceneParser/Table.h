/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TABLE_H
#define TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SceneInterface.h"

struct Table;
struct TableEnt;

extern struct Table *TblNew(void);
extern void TblFree(struct Table *table);

extern struct TableEnt *TblLookup(struct Table *table, const char *key);
extern struct TableEnt *TblAdd(struct Table *table, const char *key, ID id);

extern const char *EntGetName(const struct TableEnt *ent);
extern ID EntGetID(const struct TableEnt *ent);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

