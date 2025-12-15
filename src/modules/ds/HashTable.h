#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

#include "../../defines.h"
#include "GenericLinkedList.h"
#include "GenericDynamicArray.h"
#include "../log/log.h"

#define INITIAL_HT_SIZE 16

typedef bool (*compare_fallback_fn)(void *, const void *);
typedef u32 (*hash_fn)(const char *, u32 data_size);

typedef struct __HashTable {
  u32 data_size, key_size;
  DynamicArray *buckets;
  hash_fn h;
} HashTable;

#define Map HashTable
#define insert_ht put_ht
#define delele_ht remove_ht
#define get_ht lookup_ht

bool init_ht(HashTable **ht, u32 data_size, hash_fn h);
bool put_ht(HashTable **ht, const char *key, const void *data);
void *lookup_ht(HashTable **ht, const void *k, compare_fallback_fn f);
bool remove_ht(HashTable **ht, void *k);
bool clear_ht(HashTable **ht);

// available default hash functions
// any other hash functions must return a non-negative integer
hash_fn modulo(const char *k, u32 data_size);
hash_fn djb2(const char *k, u32 data_size);
hash_fn sdbm(const char *k, u32 data_size);

#endif
