#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

/* This is a String Key / Generic Data HashTable
 * You can define whatever node size at the init function, them proceed to have 
 * any size within for your keys (which can be defined with ... TODO)
 */

#include "../../defines.h"
#include "GenericLinkedList.h"
#include "GenericDynamicArray.h"
#include "../log/log.h"
#include "./ds_utils.h"

#define INITIAL_HT_SIZE 16
#define DEFAULT_KEY_SIZE 32
#define DEFAULT_DATA_SIZE 128

typedef u32 (*hash_fn)(char *);

typedef struct __bucket {
  char *key;
  void *data;
} bucket;

typedef struct __bucket_head {
  GenericLinkedList *head;
} bucket_head;

typedef struct __HashTable {
  u32 data_size;
  u16 max_key_size;
  DynamicArray *bucket_heads;
  hash_fn h;
} HashTable;

#define Map HashTable
#define HashMap HashTable

#define init_hash_table(ht, data_size) init_ht(ht, data_size, DEFAULT_KEY_SIZE, murmur3_32)

bool init_ht(HashTable **ht, u32 data_size, u16 max_key_size, hash_fn h);
bool put_ht(HashTable **ht, char *key, void *data);
void *lookup_ht(HashTable **ht, void *k, cmp_fn f);
bool remove_ht(HashTable **ht, void *k, cmp_fn f);
bool clear_ht(HashTable **ht);

// TODO implement these
char *get_ht_keys(HashTable **ht);
bool print_ht(HashTable **ht, print_callback_fn f);

// available default hash functions
// any other hash functions must return a uint32_t
u32 polynomial_rolling_hash_fn(char *key);
u32 djb2(char *key);
u32 murmur3_32(char *key);

#endif
