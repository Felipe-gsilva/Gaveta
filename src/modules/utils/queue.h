#pragma once 

#include "../../defines.h"
#include "../b-tree/b-tree-buf.h"

typedef struct __queue {
  struct __queue *next;
  disk_page *page;
  u16 counter;
} queue;


queue *alloc_queue(void);

void clear_queue(queue *queue);

void print_queue(queue *queue);

void push_disk_page(b_tree_buf *b, disk_page *disk_page);

disk_page *pop_disk_page(b_tree_buf *b);

disk_page *queue_search(queue *queue, u16 rrn);
