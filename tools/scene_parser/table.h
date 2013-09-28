/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TABLE_H
#define TABLE_H

#include "fj_scene_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

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

