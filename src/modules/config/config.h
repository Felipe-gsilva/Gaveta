#ifndef _CONFIG_H
#define _CONFIG_H

#include "../../defines.h"

#define BTREE_CONFIG_PATH "assets/btree.conf"

typedef struct __btree_config {
  u32 order;
} btree_config;

bool read_btree_config();

#endif
