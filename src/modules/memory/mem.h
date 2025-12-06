#ifndef _MEMORY
#define _MEMORY

#include "../../defines.h"

#define MB 1048576
#define KB 1024

#define DEFAULT_MEMORY_SIZE MB
#define PAGE_SIZE KB

typedef struct __page {
  int id;
  void *p;
  bool free;
  bool used;
} page;

typedef struct __page_table {
  u32 len;
  u32 free_page_num;
  u32 *free_stack;
  u32 free_stack_top;
  page *pages;
} page_table;

typedef struct __memory {
  sem_t memory_s;
  page_table pt;
  void *pool;
  u32 size;
  bool initialized;
} memory;

typedef struct __alloc_header {
  int id;
  u32 page_num;
} alloc_header;

typedef enum {
  CODE = 0x000,
  STACK = 0x001,
  HEAP = 0x002,
} segment;

#define USE_GLOBAL_MEMORY_POOL NULL
#define init_mem(mem_size) init_memory(USE_GLOBAL_MEMORY_POOL, mem_size)
#define clear_mem() clear_memory(USE_GLOBAL_MEMORY_POOL)
#define g_alloc(bytes) g_allocate(USE_GLOBAL_MEMORY_POOL, bytes)
#define g_realloc(curr_region, bytes) g_reallocate(USE_GLOBAL_MEMORY_POOL, curr_region, bytes)
#define g_dealloc(mem) g_deallocate(USE_GLOBAL_MEMORY_POOL, mem)

void init_memory(memory *mem_pool, u32 mem_size);

void clear_memory(memory *mem_pool);

void *g_allocate(memory *mem_pool, u32 bytes);

void *g_reallocate(memory *mem_pool, void *curr_region, u32 bytes);

alloc_header *get_header(void *ptr);

void g_deallocate(memory *mem_pool, void *mem);

void push_free_stack(memory *mem_pool, u32 i);

void print_page_table_status(memory *mem_pool);

bool is_mem_free(memory *mem_pool, void *ptr);

float retrieve_free_mem_percentage(memory *mem_pool);

float retrieve_used_mem_percentage(memory *mem_pool);

int second_chance(memory *mem_pool);

#endif
