#ifndef _CONFIG_H
#define _CONFIG_H

#include "../../defines.h"
#include "../ds/HashTable.h"

#define BTREE_CONFIG_PATH "assets/default_btree.json"

typedef struct __btree_config {
  char name[BIG_STR_BUFF_SIZE];
  char index_file[MAX_ADDRESS], idx_free_rrn_address[MAX_ADDRESS],
      data_free_rrn_address[MAX_ADDRESS], data_file[MAX_ADDRESS];
  HashTable *schema;           // not usable yet
  u32 schema_size, order; // using this as a placeholder
  u16 root_rrn, header_size;
} btree_config;

bool read_btree_config(const char *file_name, btree_config *cfg);

bool write_btree_config(const char *file_name, btree_config *cfg);

#endif
