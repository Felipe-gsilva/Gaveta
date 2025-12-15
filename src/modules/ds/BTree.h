#ifndef __BTREE_H__
#define __BTREE_H__

#include "../../defines.h"
#include "./GenericQueue.h"
#include "./GenericLinkedList.h"
#include "../config/config.h"

typedef enum {
  BTREE_INSERTED_IN_BTREE_NODE = 5,
  BTREE_NOT_FOUND_KEY = 3,
  BTREE_FOUND_KEY = 2,
  BTREE_PROMOTION = 1,
  BTREE_NO_PROMOTION = 0,
  BTREE_SUCCESS = 0,
  BTREE_ERROR_MEMORY = -1,
  BTREE_ERROR_IO = -2,
  BTREE_ERROR_DUPLICATE = -3,
  BTREE_ERROR_INVALID_BTREE_NODE = -4,
  BTREE_ERROR_BTREE_NODE_FULL = -5
} btree_status;

#define BTREE_CACHE_DEFAULT_CAPACITY 128

typedef struct __key {
  u16 data_register_rrn;
  void *id;
  u32 key_size;
} key;

typedef struct __key_range {
  key *start_id;
  key *end_id;
} key_range;

typedef struct __btree_node {
  key *keys;
  u16 rrn, *children, next_leaf;
  u8 child_num, keys_num, leaf;
  // control bytes
  byte dirty, pinned;
} btree_node;

typedef struct __data_record {
  void *data;
  key *k;
} data_record;

typedef struct __io_buf {
  char address[MAX_ADDRESS];
  FILE *fp;
  GenericLinkedList *free_rrn;
} io_buf; 

typedef struct __btree_cache {
  GenericQueue *gq;
  u32 cache_size;
  u32 cache_capacity;
} btree_cache;

typedef struct __write_req {
  u16 rrn;
  void *data;
} write_req;

typedef struct __io_wrapper {
  io_buf *index;
  io_buf *data;
  GenericQueue *write_reqs;
} io_wrapper;

typedef struct __BTree {
  btree_node *root;
  btree_config config;
  btree_cache cache;
  io_wrapper io;
} BTree;

void print_gq_btree_node(void *data);

#define create_btree(btree_config_file)  create_btree_from_file(btree_config_file, NULL)

BTree *create_btree_from_file(const char *config_file, const char* data_file);

btree_status b_insert(BTree *b, void *d, u16 rrn);

void create_index_file(BTree *b);

void clear_btree(BTree *b);

btree_node *b_search(BTree *b, const char *s, u16 *return_pos);

void b_range_search(BTree *b, key_range *range);

btree_status b_remove(BTree *b, char *key_id);

void print_btree_node(btree_node *btree_node);

btree_node *load_btree_node(BTree *b, u16 rrn);

btree_node *new_btree_node(u16 rrn);

bool clear_btree_node(btree_node *btree_node);

bool compare_btree_nodes(void *v1, void *v2);

bool clear_all_btree_nodes(void);

void track_btree_node(btree_node *p);

bool load_list(GenericLinkedList *i, char* s);

u16 get_free_rrn(GenericLinkedList *i);

io_buf *alloc_io_buf(void);

#define load_index_file(btree) load_file(btree, "index")
#define load_data_file(btree) load_file(btree, "data")
void load_file(BTree *b, const char *type);

void create_data_file(BTree *b);

void *load_data_record(BTree *b, u16 rrn);

void write_data_record(BTree *b, void *d, u16 rrn);

void clear_io_buf(io_buf *io_buf);

void d_insert(BTree *b, void *d, GenericLinkedList *free_rrn, u16 rrn);
#endif
