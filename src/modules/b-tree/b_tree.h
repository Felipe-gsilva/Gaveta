#ifndef __BTREE_H__
#define __BTREE_H__

#include "../../defines.h"

#define ORDER 20 // for now

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
  char *id;
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
}
btree_node;

typedef struct __queue {
  struct __queue *next;
  btree_node *btree_node;
  u16 counter;
} queue;

typedef struct __data_record {
  char *data;
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

typedef struct __free_rrn_list {
  io_buf *io;
  u16 *free_rrn;
  u16 n;
} free_rrn_list;

typedef struct __b_tree_buf {
  btree_node *root;
  io_buf *io;
  queue *q;
  free_rrn_list *i;
} b_tree_buf;

// procedures
queue *alloc_queue(void);

void clear_queue(queue *queue);

void print_queue(queue *queue);

void push_btree_node(b_tree_buf *b, btree_node *btree_node);

btree_node *pop_btree_node(b_tree_buf *b);

btree_node *queue_search(queue *queue, u16 rrn);
bool is_queue_empty(queue *queue);

b_tree_buf *alloc_tree_buf(void);

void build_tree(b_tree_buf *b, io_buf *data, int n);

btree_status handle_underflow(b_tree_buf *b, btree_node *p);

btree_node *get_sibling(b_tree_buf *b, btree_node *p, bool left);

btree_node *find_parent(b_tree_buf *b, btree_node *current, btree_node *target);

btree_status b_insert(b_tree_buf *b, io_buf *data, data_record *d, u16 rrn);

btree_status insert_key(b_tree_buf *b, btree_node *p, key k, key *promo_key,
                        btree_node **r_child, bool *promoted);

btree_status b_split(b_tree_buf *b, btree_node *p, btree_node **r_child, key *promo_key,
                     key *incoming_key, bool *promoted);

btree_status insert_in_btree_node(btree_node *p, key k, btree_node *r_child, int pos);

void create_index_file(io_buf *io, const char *file_name);

void clear_tree_buf(b_tree_buf *b);

int write_root_rrn(b_tree_buf *b, u16 rrn);

btree_node *b_search(b_tree_buf *b, const char *s, u16 *return_pos);

void b_range_search(b_tree_buf *b, io_buf *data, key_range *range);

u16 search_key(b_tree_buf *b, btree_node *p, key key, u16 *found_pos,
               btree_node **return_btree_node);

int search_in_btree_node(btree_node *btree_node, key key, int *return_pos);

btree_status b_remove(b_tree_buf *b, io_buf *data, char *key_id);

btree_status remove_key(b_tree_buf *b, btree_node *p, key k, bool *merged);

btree_status redistribute(b_tree_buf *b, btree_node *donor, btree_node *receiver, bool from_left);

btree_status merge(b_tree_buf *b, btree_node *left, btree_node *right);

void print_btree_node(btree_node *btree_node);

btree_node *load_btree_node(b_tree_buf *b, u16 rrn);

void populate_index_header(index_header_record *bh, const char *file_name);

void load_index_header(io_buf *io);

int write_index_header(io_buf *io);

int write_index_record(b_tree_buf *b, btree_node *p);

btree_node *alloc_btree_node(void);

btree_node *new_btree_node(u16 rrn);

void clear_btree_node(btree_node *btree_node);

void clear_all_btree_nodes(void);

void track_btree_node(btree_node *p);
free_rrn_list *alloc_ilist(void);

void clear_ilist(free_rrn_list *i);

void load_list(free_rrn_list *i, char* s);

u16 *load_rrn_list(free_rrn_list *i);

u16 get_free_rrn(free_rrn_list *i);

u16 get_last_free_rrn(free_rrn_list *i);

u16 *load_rrns(free_rrn_list *i);

void insert_list(free_rrn_list *i, int rrn);

io_buf *alloc_io_buf(void);

void print_data_record(data_record *hr);

void load_file(io_buf *io, char *file_name, const char *type);

void create_data_file(io_buf *io, char *file_name);

void load_data_header(io_buf *io);

data_record *load_data_record(io_buf *io, u16 rrn);

void populate_header(data_header_record *hp, const char *file_name);

void prepend_data_header(io_buf *io);

void write_data_header(io_buf *io);

void write_data_record(io_buf *io, data_record *d, u16 rrn);

void clear_io_buf(io_buf *io_buf);

void d_insert(io_buf *io, data_record *d, free_rrn_list *ld, u16 rrn);


#endif
