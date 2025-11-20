#pragma once
#include "free-rrn-list.h"
#include "../../defines.h"
#include "../utils/queue.h"

typedef struct __b_tree_buf {
  disk_page *root;
  io_buf *io;
  queue *q;
  free_rrn_list *i;
} b_tree_buf;

b_tree_buf *alloc_tree_buf(void);

void build_tree(b_tree_buf *b, io_buf *data, int n);

btree_status handle_underflow(b_tree_buf *b, disk_page *p);

disk_page *get_sibling(b_tree_buf *b, disk_page *p, bool left);

disk_page *find_parent(b_tree_buf *b, disk_page *current, disk_page *target);

btree_status b_insert(b_tree_buf *b, io_buf *data, data_record *d, u16 rrn);

btree_status insert_key(b_tree_buf *b, disk_page *p, key k, key *promo_key,
                        disk_page **r_child, bool *promoted);

btree_status b_split(b_tree_buf *b, disk_page *p, disk_page **r_child, key *promo_key,
                     key *incoming_key, bool *promoted);

btree_status insert_in_disk_page(disk_page *p, key k, disk_page *r_child, int pos);

void create_index_file(io_buf *io, const char *file_name);

void clear_tree_buf(b_tree_buf *b);

int write_root_rrn(b_tree_buf *b, u16 rrn);

disk_page *b_search(b_tree_buf *b, const char *s, u16 *return_pos);

void b_range_search(b_tree_buf *b, io_buf *data, key_range *range);

u16 search_key(b_tree_buf *b, disk_page *p, key key, u16 *found_pos,
               disk_page **return_disk_page);

int search_in_disk_page(disk_page *disk_page, key key, int *return_pos);

btree_status b_remove(b_tree_buf *b, io_buf *data, char *key_id);

btree_status remove_key(b_tree_buf *b, disk_page *p, key k, bool *merged);

btree_status redistribute(b_tree_buf *b, disk_page *donor, disk_page *receiver, bool from_left);

btree_status merge(b_tree_buf *b, disk_page *left, disk_page *right);

void print_disk_page(disk_page *disk_page);

disk_page *load_disk_page(b_tree_buf *b, u16 rrn);

void populate_index_header(index_header_record *bh, const char *file_name);

void load_index_header(io_buf *io);

int write_index_header(io_buf *io);

int write_index_record(b_tree_buf *b, disk_page *p);

disk_page *alloc_disk_page(void);

disk_page *new_disk_page(u16 rrn);

void clear_disk_page(disk_page *disk_page);

void clear_all_disk_pages(void);

void track_disk_page(disk_page *p);
