#ifndef _CONFIG_H
#define _CONFIG_H

#include "../../defines.h"

#define BTREE_CONFIG_PATH "assets/default_btree.json"

typedef struct __btree_config {
  u32 order;
  char index_file[MAX_ADDRESS];
  char data_file[MAX_ADDRESS];
  void *schema; // not usable yet
  u32 schema_size; // using this as a placeholder
} btree_config;

bool read_btree_config(const char *file_name, btree_config *cfg);

#endif
