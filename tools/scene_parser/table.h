// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef TABLE_H
#define TABLE_H

#include "fj_scene_interface.h"
#include <string>

class Table;
class TableEnt;

extern Table *TblNew(void);
extern void TblFree(Table *table);

extern TableEnt *TblLookup(Table *table, const char *key);
extern TableEnt *TblAdd(Table *table, const char *key, fj::ID id);

extern const std::string &EntGetName(const TableEnt *ent);
extern fj::ID EntGetID(const TableEnt *ent);

#endif // FJ_XXX_H
