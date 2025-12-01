#ifndef __GENERIC_QUEUE_H
#define __GENERIC_QUEUE_H

#include "../../defines.h"
#include "../memory/mem.h"

typedef struct __generic_queue {
  void *data;
  void *next;
  void *last;
  u32 data_size;
} generic_queue;

typedef struct __generic_queue_table  {
  generic_queue *head;
  generic_queue *tail;
  int size;
  int capability;
} generic_queue_table;

bool init_generic_queue(generic_queue **gq, u32 data_size);
bool push_generic_queue(generic_queue **gq, void *data);
bool pop_generic_queue(generic_queue **gq, generic_queue *save_to);

generic_queue *top_generic_queue(generic_queue **gq);
bool is_generic_queue_empty(generic_queue **gq);
void print_generic_queue(generic_queue **gq);

#endif
