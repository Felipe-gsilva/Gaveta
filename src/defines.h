#ifndef _DEFINES_H
#define _DEFINES_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

// in bytes
#define MAX_ADDRESS 4096

// queue max
#define P 5 

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct __index_header_record index_header_record;
typedef struct __data_header_record data_header_record;
typedef struct __io_buf io_buf;
typedef struct __index_record index_record;
typedef struct __data_record data_record;
typedef struct __key key;
typedef struct __key_range key_range;
typedef struct __btree_node btree_node;
typedef struct __free_rrn_list free_rrn_list;

struct __key {
  u16 data_register_rrn;
  char *id;
};

struct __key_range {
  char *start_id;
  char *end_id;
};

struct __btree_node {
  key *keys;
  u16 rrn;
  u16 *children;
  u16 next_leaf;
  u8 child_num;
  u8 keys_num;
  u8 leaf;
};


struct __data_record {
  char *data;
  char *key;
  u16 rrn;
};

struct __data_header_record {
  u16 header_size;
  u16 record_size;
  char *free_rrn_address;
};

struct __index_header_record {
  u16 root_rrn;
  u16 page_size;
  u16 header_size;
  char *free_rrn_address;
};

struct __io_buf {
  char address[MAX_ADDRESS];
  FILE *fp;
  data_header_record *hr;
  index_header_record *br;
};


struct __free_rrn_list {
  io_buf *io;
  u16 *free_rrn;
  u16 n;
};


#endif
