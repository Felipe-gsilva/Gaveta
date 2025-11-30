#ifndef __GENERIC_QUEUE_H
#define __GENERIC_QUEUE_H

#include "../../defines.h"
#include "../memory/mem.h"

typedef struct __generic_queue_entry {
  void *data;
  void *next;
  int data_size;
} generic_queue_entry;

typedef union __generic_queue {
  int __align;
  generic_queue_entry queue;
} generic_queue;

#define compare(gq, k) (gq->queue->data_size == k)

bool init_generic_queue(generic_queue **gq, int data_size);
bool push_generic_queue(generic_queue **gq, void *data);
bool pop_generic_queue(generic_queue **gq);
bool is_queue_empty(generic_queue **gq);

#endif
