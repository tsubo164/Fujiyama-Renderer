/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Table.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

enum { HASH_SIZE = 1237 }; /* a prime number */
enum { MULTIPLIER = 31 };

static struct TableEnt *EntNew(const char *key, ID id);
static void EntFree(struct TableEnt *ent);

static unsigned int hash_fn(const char *key);
static char * str_dup(const char *s);

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
	struct Table *table;

	table = (struct Table *) malloc(sizeof(struct Table));
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
	struct TableEnt *next, *kill;

	if (table == NULL)
		return;

	for (i = 0; i < HASH_SIZE; i++) {
		if (table->table[i] == NULL)
			continue;
		for (next = table->table[i]; next != NULL; ) {
			kill = next;
			next = next->next;
			EntFree(kill);
		}
	}

	free(table);
}

struct TableEnt *TblLookup(struct Table *table, const char *key)
{
	int h;
	struct TableEnt *ent;

	h = hash_fn(key);
	for (ent = table->table[h]; ent != NULL; ent = ent->next) {
		if (strcmp(key, ent->key) == 0) {
			break;
		}
	}

	return ent;
}

struct TableEnt *TblAdd(struct Table *table, const char *key, ID id)
{
	int h;
	struct TableEnt *ent;

	h = hash_fn(key);
	for (ent = table->table[h]; ent != NULL; ent = ent->next) {
		if (strcmp(key, ent->key) == 0) {
			return ent;
		}
	}

	assert(ent == NULL);
	ent = EntNew(key, id);
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

static struct TableEnt *EntNew(const char *key, ID id)
{
	struct TableEnt *ent;

	ent = (struct TableEnt *) malloc(sizeof(struct TableEnt));
	if (ent == NULL)
		return NULL;

	ent->key = str_dup(key);
	if (ent->key == NULL) {
		EntFree(ent);
		return NULL;
	}

	ent->id = id;
	ent->next = NULL;
	return ent;
}

static void EntFree(struct TableEnt *ent)
{
	if (ent == NULL)
		return;

	if (ent->key != NULL)
		free(ent->key);

	free(ent);
}

static unsigned int hash_fn(const char *key)
{
	unsigned int h = 0;
	unsigned char *p = NULL;

	for (p = (unsigned char *) key; *p != '\0'; p++)
		h = MULTIPLIER * h + *p;

	return h % HASH_SIZE;
}

static char * str_dup(const char *s)
{
	size_t alloc;
	char *dup;

	alloc = strlen(s) + 1;
	dup = (char *) malloc(sizeof(char) * alloc);
	if (dup == NULL)
		return NULL;

	strcpy(dup, s);
	return dup;
}

