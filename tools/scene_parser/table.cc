// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "table.h"

#include <cstring>
#include <cassert>

using namespace fj;

enum { HASH_SIZE = 1237 }; // a prime number
enum { MULTIPLIER = 31 };

static TableEnt *new_entry(const char *key, ID id);
static void free_entry(TableEnt *ent);

static unsigned int hash_fn(const char *key);

class TableEnt {
public:
  TableEnt(std::string Key, ID Id) : key(Key), id(Id), next(NULL) {}
  ~TableEnt() {}

public:
  std::string key;
  ID id;
  TableEnt *next;
};

class Table {
public:
  Table();
  ~Table();

public:
  TableEnt *table_[HASH_SIZE];
};

Table::Table()
{
  for (int i = 0; i < HASH_SIZE; i++) {
    table_[i] = NULL;
  }
}

Table::~Table()
{
  for (int i = 0; i < HASH_SIZE; i++) {
    for (TableEnt *ent = table_[i]; ent != NULL; ) {
      // need to save the entry to be freed before moving to next
      TableEnt *kill = ent;
      ent = ent->next;
      free_entry(kill);
    }
  }
}

Table *TblNew(void)
{
  return new Table();
}

void TblFree(Table *table)
{
  delete table;
}

TableEnt *TblLookup(Table *table, const char *key)
{
  TableEnt *ent = NULL;
  const unsigned int h = hash_fn(key);

  for (ent = table->table_[h]; ent != NULL; ent = ent->next) {
    if (ent->key == key) {
      break;
    }
  }

  return ent;
}

TableEnt *TblAdd(Table *table, const char *key, ID id)
{
  TableEnt *ent = NULL;
  const unsigned int h = hash_fn(key);

  for (ent = table->table_[h]; ent != NULL; ent = ent->next) {
    if (ent->key == key) {
      return ent;
    }
  }

  assert(ent == NULL);
  ent = new_entry(key, id);
  if (ent == NULL)
    return NULL;

  ent->next = table->table_[h];
  table->table_[h] = ent;

  return ent;
}

const std::string &EntGetName(const TableEnt *ent)
{
  return ent->key;
}

ID EntGetID(const TableEnt *ent)
{
  return ent->id;
}

static TableEnt *new_entry(const char *key, ID id)
{
  return new TableEnt(key, id);
}

static void free_entry(TableEnt *ent)
{
  delete ent;
}

static unsigned int hash_fn(const char *key)
{
  unsigned char *p = NULL;
  unsigned int h = 0;

  for (p = (unsigned char *) key; *p != '\0'; p++)
    h = MULTIPLIER * h + *p;

  return h % HASH_SIZE;
}
