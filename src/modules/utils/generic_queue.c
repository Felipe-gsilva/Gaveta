#include "generic_queue.h"
#include "../memory/mem.h"
#include "../log/log.h"

static generic_queue *create_generic_queue_entry(void* data, u32 data_size){
  generic_queue *gq = g_alloc(sizeof(generic_queue));

  gq->data_size = data_size;
  gq->next = NULL;
  gq->last = NULL;
  if (data != NULL) {
    gq->data = g_alloc(data_size);
    memcpy(gq->data, data, data_size);
  }
  return gq;
}

bool init_generic_queue(generic_queue **gq, u32 data_size) {
  assert(gq && data_size > 0);
  (*gq) = create_generic_queue_entry(NULL, data_size);
  return true;
}


bool push_generic_queue(generic_queue **gq, void *data) {
  if (!gq || *gq)  {
    g_error(QUEUE_EMPTY, "Invalid pointer passed as queue");
    return false;
  }


  generic_queue *new = create_generic_queue_entry(data, (*gq)->data_size);

  if (!(*gq)) {
    *gq = new;
    return true;
  }

  generic_queue *aux = *gq;

  while (aux->next != NULL)
    aux = aux->next;

  aux->next = new;
  return true;
}

bool pop_generic_queue(generic_queue **gq, generic_queue *save_to){
  assert(gq && *gq);

  if (save_to) memcpy(save_to, *gq, sizeof(*gq));
  generic_queue *aux = (*gq)->next;
  g_dealloc(*gq);
  *gq = aux;
  return true;
}

bool is_queue_empty(generic_queue **gq){
  if ((*gq)->data) {
    return false;
  }
  return true;
}

generic_queue *top_generic_queue(generic_queue **gq) {
  if (is_queue_empty(gq))  {
    g_error(QUEUE_EMPTY, "Trying to top a empty generic queue!");
    return NULL;
  }
  
  return (*gq);
}
