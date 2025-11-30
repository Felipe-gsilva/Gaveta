#include "generic_queue.h"


bool init_generic_queue(generic_queue **gq, int data_size) {
  if (*gq) {
    *gq = g_alloc(sizeof(generic_queue));
  }

  (*gq)->__align = 0;
  (*gq)->queue.next = NULL;
  (*gq)->queue.data = NULL;
  (*gq)->queue.data_size = data_size;

  printf("align: %d",  (*gq)->__align);
  return true;
}

bool push_generic_queue(generic_queue **gq, void *data){
  return true;
}
bool pop_generic_queue(generic_queue **gq){
  return true;
}
bool is_queue_empty(generic_queue **gq){
  if ((*gq)->__align != 0) {
    return false;
  }
  return true;
}
