#ifndef __BTREE_H__
#define __BTREE_H__

#include "../../defines.h"
#include "./GenericQueue.h"
#include "./GenericLinkedList.h"

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
  BTREE_ERROR_INVALID_btree_node = -4,
  BTREE_ERROR_BTREE_NODE_FULL = -5
} btree_status;

// structs
typedef struct __key {
  u16 data_register_rrn;
  void *id;
} key;

typedef struct __key_range {
  key *start_id;
  key *end_id;
} key_range;

typedef struct __btree_node {
  key *keys;
  u16 rrn;
  u16 *children;
  u16 next_leaf;
  u8 child_num;
  u8 keys_num;
  u8 leaf;
} btree_node;

typedef struct __data_record {
  void *data;
  char *key;
  u16 rrn;
} data_record;

typedef struct __data_header_record {
  u16 header_size;
  u16 record_size;
  char *free_rrn_address;
} data_header_record;

typedef struct __index_header_record {
  u16 root_rrn;
  u16 btree_node_size;
  u16 header_size;
  char *free_rrn_address;
} index_header_record;

typedef struct __io_buf {
  char address[MAX_ADDRESS];
  FILE *fp;
  data_header_record *hr;
  index_header_record *br;
} io_buf;

typedef struct __BTree {
  btree_node *root;
  io_buf *idx;
  io_buf *data;
  GenericQueue *q;
  GenericLinkedList *free_rrn;
  u32 order;
} BTree;

void print_gq_btree_node(void *data);

BTree *alloc_tree_buf(u32 order);

BTree *create_btree(const char *config_file);

bool build_tree(BTree *b, u32 n);

btree_status handle_underflow(BTree *b, btree_node *p);

btree_node *get_sibling(BTree *b, btree_node *p, bool left);

btree_node *find_parent(BTree *b, btree_node *current, btree_node *target);

btree_status b_insert(BTree *b, void *d, u16 rrn);

btree_status insert_key(BTree *b, btree_node *p, key k, key *promo_key,
                        btree_node **r_child, bool *promoted);

btree_status b_split(BTree *b, btree_node *p, btree_node **r_child, key *promo_key,
                     key *incoming_key, bool *promoted);

btree_status insert_in_btree_node(btree_node *p, key k, btree_node *r_child, int pos);

void create_index_file(io_buf *io, const char *file_name);

void clear_tree_buf(BTree *b);

int write_root_rrn(BTree *b, u16 rrn);

btree_node *b_search(BTree *b, const char *s, u16 *return_pos);

void b_range_search(BTree *b, key_range *range);

u16 search_key(BTree *b, btree_node *p, key key, u16 *found_pos,
               btree_node **return_btree_node);

int search_in_btree_node(btree_node *btree_node, key key, int *return_pos);

btree_status b_remove(BTree *b, char *key_id);

btree_status remove_key(BTree *b, btree_node *p, key k, bool *merged);

btree_status redistribute(BTree *b, btree_node *donor, btree_node *receiver, bool from_left);

btree_status merge(BTree *b, btree_node *left, btree_node *right);

void print_btree_node(btree_node *btree_node);

btree_node *load_btree_node(BTree *b, u16 rrn);

void populate_index_header(index_header_record *bh, const char *file_name);

void load_index_header(io_buf *io);

int write_index_header(io_buf *io);

int write_index_record(BTree *b, btree_node *p);

btree_node *alloc_btree_node(u32 order);

btree_node *new_btree_node(u16 rrn);

bool clear_btree_node(btree_node *btree_node);

bool compare_btree_nodes(void *v1, void *v2);

bool clear_all_btree_nodes(void);

void track_btree_node(btree_node *p);

bool load_list(GenericLinkedList *i, char* s);

u16 get_free_rrn(GenericLinkedList *i);

u16 get_last_free_rrn(GenericLinkedList *i);

u16 *load_rrns(GenericLinkedList *i);

io_buf *alloc_io_buf(void);

void load_file(io_buf *io, char *file_name, const char *type);

void create_data_file(io_buf *io, char *file_name);

void load_data_header(io_buf *io);

void *load_data_record(io_buf *io, u16 rrn);

void populate_header(data_header_record *hp, const char *file_name);

void prepend_data_header(io_buf *io);

void write_data_header(io_buf *io);

void write_data_record(io_buf *io, void *d, u16 rrn);

void clear_io_buf(io_buf *io_buf);

void d_insert(io_buf *data, void *d, GenericLinkedList *free_rrn, u16 rrn);
#endif
