#ifndef _APP
#define _APP

#include "modules/memory/mem.h"
#include "modules/log/log.h"
#include "modules/ds/BTree.h"
#include "modules/utils/utils.h"
#include "modules/config/config.h"
#include "modules/ds/GenericDynamicArray.h"

typedef struct __App  {
  io_buf *idx;
  io_buf *data;
  DynamicArray *btrees;
} App;

void cli();

void init_app(void);

void clear_app();

int main(int argc, char **argv);

#endif
