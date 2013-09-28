/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "table.h"
#include "fj_string_function.h"
#include "fj_memory.h"

#include <string.h>
#include <assert.h>

enum { HASH_SIZE = 1237 }; /* a prime number */
enum { MULTIPLIER = 31 };

static struct TableEnt *new_entry(const char *key, ID id);
static void free_entry(struct TableEnt *ent);

static unsigned int hash_fn(const char *key);

struct TableEnt {
  char *key;
  ID id;
  struct TableEnt *next;
};

struct Table {
  struct TableEnt *table[HASH_SIZE];
};

struct Table *TblNew(void)
{
  int i;
  struct Table *table = SI_MEM_ALLOC(struct Table);

  if (table == NULL)
    return NULL;

  for (i = 0; i < HASH_SIZE; i++) {
    table->table[i] = NULL;
  }

  return table;
}

void TblFree(struct Table *table)
{
  int i;
  struct TableEnt *ent = NULL;

  if (table == NULL)
    return;

  for (i = 0; i < HASH_SIZE; i++) {
    if (table->table[i] == NULL)
      continue;
    for (ent = table->table[i]; ent != NULL; ) {
      /* need to save the entry to be freed before moving to next */
      struct TableEnt *kill = ent;
      ent = ent->next;
      free_entry(kill);
    }
  }

  SI_MEM_FREE(table);
}

struct TableEnt *TblLookup(struct Table *table, const char *key)
{
  struct TableEnt *ent = NULL;
  const unsigned int h = hash_fn(key);

  for (ent = table->table[h]; ent != NULL; ent = ent->next) {
    if (strcmp(key, ent->key) == 0) {
      break;
    }
  }

  return ent;
}

struct TableEnt *TblAdd(struct Table *table, const char *key, ID id)
{
  struct TableEnt *ent = NULL;
  const unsigned int h = hash_fn(key);

  for (ent = table->table[h]; ent != NULL; ent = ent->next) {
    if (strcmp(key, ent->key) == 0) {
      return ent;
    }
  }

  assert(ent == NULL);
  ent = new_entry(key, id);
  if (ent == NULL)
    return NULL;

  ent->next = table->table[h];
  table->table[h] = ent;

  return ent;
}

const char *EntGetName(const struct TableEnt *ent)
{
  return ent->key;
}

ID EntGetID(const struct TableEnt *ent)
{
  return ent->id;
}

static struct TableEnt *new_entry(const char *key, ID id)
{
  struct TableEnt *ent = SI_MEM_ALLOC(struct TableEnt);

  if (ent == NULL)
    return NULL;

  ent->key = StrDup(key);
  if (ent->key == NULL) {
    free_entry(ent);
    return NULL;
  }

  ent->id = id;
  ent->next = NULL;
  return ent;
}

static void free_entry(struct TableEnt *ent)
{
  if (ent == NULL)
    return;

  ent->key = StrFree(ent->key);
  SI_MEM_FREE(ent);
}

static unsigned int hash_fn(const char *key)
{
  unsigned char *p = NULL;
  unsigned int h = 0;

  for (p = (unsigned char *) key; *p != '\0'; p++)
    h = MULTIPLIER * h + *p;

  return h % HASH_SIZE;
}

