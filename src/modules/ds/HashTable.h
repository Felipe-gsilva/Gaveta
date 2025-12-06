#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

#include "../../defines.h"
#include "GenericLinkedList.h"
#include "GenericDynamicArray.h"
#include "../log/log.h"

#define hash_fn void*
#define key void*
#define value void*
#define INITIAL_HT_SIZE 16
#define DEFAULT_HASH_FN (hash_fn)(void *, void*)

typedef struct __HashTable {
  u32 data_size;
  u32 key_size;
  DynamicArray **val;
  hash_fn (*h)(key);
} HashTable;

bool init_ht(HashTable **ht, u32 data_size, hash_fn (h)(key));
bool insert_ht(HashTable **ht, value data);
bool lookup_ht(HashTable **ht, key k);
bool clear_ht(HashTable **ht);

#endif
