#ifndef _DEFINES_H
#define _DEFINES_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ORDER 5

// in bytes
#define MAX_ADDRESS 4096

// size data record
#define TAMANHO_PLACA 8
#define TAMANHO_MODELO 20
#define TAMANHO_MARCA 20
#define TAMANHO_CATEGORIA 15
#define TAMANHO_STATUS 16

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

typedef enum {
  BTREE_INSERTED_IN_PAGE = 5,
  BTREE_NOT_FOUND_KEY = 3,
  BTREE_FOUND_KEY = 2,
  BTREE_PROMOTION = 1,
  BTREE_NO_PROMOTION = 0,
  BTREE_SUCCESS = 0,
  BTREE_ERROR_MEMORY = -1,
  BTREE_ERROR_IO = -2,
  BTREE_ERROR_DUPLICATE = -3,
  BTREE_ERROR_INVALID_PAGE = -4,
  BTREE_ERROR_PAGE_FULL = -5
} btree_status;

typedef enum {  // not integrated yet
  IO_SUCCESS = 0,
  IO_ERROR = -1
} io_status;

typedef struct __index_header_record index_header_record;
typedef struct __data_header_record data_header_record;
typedef struct __b_tree_buf b_tree_buf;
typedef struct __io_buf io_buf;
typedef struct __index_record index_record;
typedef struct __data_record data_record;
typedef struct __queue queue;
typedef struct __key key;
typedef struct __key_range key_range;
typedef struct __page mem_page;
typedef struct __App App;
typedef struct __free_rrn_list free_rrn_list;


struct __key {
  u16 data_register_rrn;
  char id[TAMANHO_PLACA];
};

struct __key_range {
  char start_id[TAMANHO_PLACA];
  char end_id[TAMANHO_PLACA];
};

struct __page {
  key keys[ORDER - 1];
  u16 rrn;
  u16 children[ORDER];
  u16 next_leaf;
  u8 child_num;
  u8 keys_num;
  u8 leaf;
};

struct __queue {
  queue *next;
  mem_page *page;
  u16 counter;
};

struct __data_record {
  char placa[TAMANHO_PLACA];
  char modelo[TAMANHO_MODELO];
  char marca[TAMANHO_MARCA];
  int ano;
  char categoria[TAMANHO_CATEGORIA];
  int quilometragem;
  char status[TAMANHO_STATUS];
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

struct __b_tree_buf {
  mem_page *root;
  io_buf *io;
  queue *q;
  free_rrn_list *i;
};

struct __free_rrn_list {
  io_buf *io;
  u16 *free_rrn;
  u16 n;
};

struct __App  {
  io_buf *idx;
  io_buf *data;
  b_tree_buf *b;
  free_rrn_list *ld;
  bool debug;
  memory *mem;
};

#endif
