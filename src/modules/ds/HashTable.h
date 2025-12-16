#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

#include "../../defines.h"
#include "GenericLinkedList.h"
#include "GenericDynamicArray.h"
#include "../log/log.h"

#define INITIAL_HT_SIZE 16

typedef u32 (*hash_fn)(const char *);

typedef struct __ht_bucket {
  void *data, *key;
  u32 key_size, data_size;
  GenericLinkedList *head;
} ht_bucket;

typedef struct __HashTable {
  u32 data_size, key_size;
  DynamicArray *buckets;
  hash_fn h;
} HashTable;

#define Map HashTable
#define insert_ht put_ht
#define delele_ht remove_ht
#define get_ht lookup_ht

#define init_default_ht(ht, data_size) init_ht(ht, data_size, polynomial_rolling_hash_fn)
bool init_ht(HashTable **ht, hash_fn h); // TODO think about
bool put_ht(HashTable **ht, const char *key, void *data);
void *lookup_ht(HashTable **ht, void *k, cmp_fn f);
bool remove_ht(HashTable **ht, void *k, cmp_fn f);
bool clear_ht(HashTable **ht);

// available default hash functions
// any other hash functions must return a non-negative integer
u32 polynomial_rolling_hash_fn(const char *key);

#endif
