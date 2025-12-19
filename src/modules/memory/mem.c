#include "mem.h"
#include "../log/log.h"

memory *global_mem_pool;

void init_memory(memory *mem_pool, u32 mem_size) {
  memory **target_mem_pool = &global_mem_pool;
  if (mem_pool) {
    target_mem_pool = &mem_pool;
  }

  if (mem_size <= 16 * KB) {
    g_error(MEM_ERROR,
            "Given memory size %d is broken! Setting to default mem size: %",
            mem_size, DEFAULT_MEMORY_SIZE);
    mem_size = DEFAULT_MEMORY_SIZE;
  }

  memory *mem = malloc(sizeof(memory));
  void *memory_pool = malloc(mem_size);

  if (!mem || !memory_pool) {
    g_crit_error(MEM_ERROR, "Could not bootstrap memory! given mem_size: %d",
                 mem_size);
  }

  mem->pt.len = mem_size / PAGE_SIZE;
  mem->pt.pages = malloc(sizeof(page) * mem->pt.len);
  mem->pt.free_stack = malloc(sizeof(u32) * mem->pt.len);
  mem->pt.free_stack_top = 0;
  mem->pt.free_page_num = mem->pt.len;
  mem->pool = memory_pool;
  mem->size = mem_size;
  *target_mem_pool = mem;

  for (u32 i = 0; i < mem->pt.len; i++) {
    mem->pt.pages[i].id = i;
    mem->pt.pages[i].p = memory_pool + (i * PAGE_SIZE);
    mem->pt.pages[i].free = true;
    mem->pt.pages[i].used = false;
    push_free_stack(*target_mem_pool, i);
  }

  if (!mem->pt.pages) {
    g_crit_error(MEM_ERROR, "Could not allocate pages!");
  }

  if (sem_init(&(*target_mem_pool)->memory_s, 0, 1) != 0) {
    g_crit_error(SEMAPHORE_INIT_ERROR, "Memory semaphore failed to initialize")
  }

  (*target_mem_pool)->initialized = true;
  g_info("%dB allocated for the app pool", (int)mem_size);
}

void clear_memory(memory *mem_pool) {
  memory **target_mem_pool = &global_mem_pool;
  if (mem_pool) target_mem_pool = &mem_pool;

  int mem_size = (*target_mem_pool)->size;

  free((*target_mem_pool)->pt.pages);
  free((*target_mem_pool)->pt.free_stack);
  free((*target_mem_pool)->pool);
  free(*target_mem_pool);

  g_info("%dB deallocated", mem_size);
}

void *g_allocate(memory *mem_pool, u32 bytes) {
  memory **target_mem_pool = &global_mem_pool;
  if (mem_pool) target_mem_pool = &mem_pool;


  if (!target_mem_pool || !*target_mem_pool) {
    g_crit_error(MEM_ALLOC_FAIL, "Memory pool not initialized!");
    return NULL;
  }

  if (bytes == 0)
    return NULL;

  u32 num_pages = (bytes + sizeof(alloc_header) + PAGE_SIZE - 1) / PAGE_SIZE;

  if (bytes >= (*target_mem_pool)->size) {
    g_error(MEM_ALLOC_FAIL,
            "greater chunk of memory than physical memory holds");
    return NULL;
  }

  if (num_pages > (*target_mem_pool)->pt.free_page_num || num_pages > (*target_mem_pool)->pt.len) {
    g_warn(MEM_STATUS, "Not enough memory to allocate %d pages", num_pages);
    while (num_pages > (*target_mem_pool)->pt.free_page_num) {
      int not_used_page = second_chance(*target_mem_pool);
      g_debug(MEM_STATUS, "Not used page %d", not_used_page);
      if (not_used_page == -1) {
        g_error(MEM_FULL, "Memory full even after page replacement!");
        sem_post(&(*target_mem_pool)->memory_s);
        return NULL;
      }

      sem_wait(&(*target_mem_pool)->memory_s);
      (*target_mem_pool)->pt.pages[not_used_page].free = true;
      (*target_mem_pool)->pt.free_page_num++;
      sem_post(&(*target_mem_pool)->memory_s);
      push_free_stack(*target_mem_pool, not_used_page);
    }
  }

  sem_wait(&(*target_mem_pool)->memory_s);
  void *ptr = NULL;
  for (u32 i = 0; i < (*target_mem_pool)->pt.len; i++) {
    bool contiguos_region = true;
    if (i + num_pages > (*target_mem_pool)->pt.len) {
      break;
    }
    for (u32 j = i; j < i + num_pages; j++) {
      if (!(*target_mem_pool)->pt.pages[j].free) {
        contiguos_region = false;
        break;
      }
    }

    if (!contiguos_region)
      continue;
    ptr = (void*)((char*)(*target_mem_pool)->pool + (i * PAGE_SIZE));
    (*target_mem_pool)->pt.free_page_num -= num_pages;
    for (u32 j = i; j < i + num_pages; j++)
    {
      (*target_mem_pool)->pt.pages[j].free = false;
      (*target_mem_pool)->pt.pages[j].used = true;
    }
    alloc_header* h_ptr = ptr;
    h_ptr->id = i;
    h_ptr->page_num = num_pages;
    break;
  }

  if (!ptr) {
    sem_post(&(*target_mem_pool)->memory_s);
    g_crit_error(MEM_ALLOC_FAIL,
                 "Failed to allocate %d bytes of memory with %d pages", bytes,
                 num_pages);
    return NULL;
  }

  sem_post(&(*target_mem_pool)->memory_s);
  return (void *)(char *)ptr + sizeof(alloc_header);
}

void *g_reallocate(memory *mem_pool, void *curr_region, u32 bytes) {
  memory **target_mem_pool = &global_mem_pool;
  if (mem_pool) target_mem_pool = &mem_pool;

  if (!curr_region) {
    g_error(MEM_REALLOC_FAIL, "NULL pointer passed to realloc");
    return NULL;
  }

  alloc_header *h_ptr = get_header(curr_region);
  if (!h_ptr) {
    g_error(MEM_REALLOC_FAIL, "No valid header found for memory: %p",
            curr_region);
    return NULL;
  }

  u32 old_size = h_ptr->page_num * PAGE_SIZE - sizeof(alloc_header);
  u32 total_size = old_size + bytes;

  void *buffer = g_allocate((*target_mem_pool), total_size);
  if (!buffer) {
    g_error(MEM_REALLOC_FAIL, "failed to realloc %d + %d bytes", old_size,
            bytes);
    return NULL;
  }

  memcpy(buffer, curr_region, old_size);
  g_deallocate((*target_mem_pool), curr_region);

  g_debug(MEM_STATUS, "region %p reallocated %d to %d bytes", buffer, old_size, bytes);

  return buffer;
}

// this function pushes a free page index onto the free stack
void push_free_stack(memory *mem_pool, u32 i) {
  if (!mem_pool) return;

  if (mem_pool->pt.free_stack_top < mem_pool->pt.len) {
    mem_pool->pt.free_stack[mem_pool->pt.free_stack_top++] = i;
  }
}

void g_deallocate(memory *mem_pool, void *mem) {
  memory **target_mem_pool = &global_mem_pool;
  if (mem_pool) target_mem_pool = &mem_pool;

  sem_wait(&(*target_mem_pool)->memory_s);
  if (!mem) {
    g_error(MEM_DEALLOC_FAIL,
            "Trying to deallocate a non allocated memory region");
    sem_post(&(*target_mem_pool)->memory_s);
    return;
  }

  alloc_header *h_ptr = get_header(mem);

  for (u32 i = h_ptr->id; i < h_ptr->id + h_ptr->page_num; i++) {
    (*target_mem_pool)->pt.pages[i].free = true;
    push_free_stack(mem_pool, i);
  }

  (*target_mem_pool)->pt.free_page_num += h_ptr->page_num;
  sem_post(&(*target_mem_pool)->memory_s);
}

alloc_header *get_header(void *ptr) {
  return (alloc_header *)((char *)ptr - sizeof(alloc_header));
}

float retrieve_free_mem_percentage(memory *mem_pool) {
  return (float)mem_pool->pt.free_page_num / (float)mem_pool->pt.len * 100.0f;
}

float retrieve_used_mem_percentage(memory *mem_pool) {
  return 100.0f - retrieve_free_mem_percentage(mem_pool);
}
void print_page_table_status(memory *mem_pool) {
  for (u32 i = 0; i < mem_pool->pt.len; i++) {
    if (mem_pool->pt.pages[i].p)
      g_info("%d %d", i, mem_pool->pt.pages[i].id);
  }
}

bool is_mem_free(memory *mem_pool, void *ptr) {
  if (!ptr)
    return true;

  sem_wait(&mem_pool->memory_s);

  alloc_header *h_ptr = get_header(ptr);
  bool is_free = true;
  for (u32 i = 0; i < h_ptr->page_num; i++) {
    page *p = (page *)(char *)mem_pool->pool + (i * PAGE_SIZE);
    if (!p->free) {
      is_free = false;
      break;
    }
  }
  sem_post(&mem_pool->memory_s);
  return is_free;
}

int second_chance(memory *mem_pool) {
  static int i = 0;
  int curr = i;

  sem_wait(&mem_pool->memory_s);
  do {
    // g_info("Page %d", curr);
    page *p = &mem_pool->pt.pages[curr];
    if (!(p->used)) {
      p->used = false;
      sem_post(&mem_pool->memory_s);
      return (i = (curr + 1) % mem_pool->pt.len);
    }
    p->used = false;
    curr = (curr + 1) %
           mem_pool->pt.len; // Atualiza o valor da curr para a próxima página
  } while (curr != i - 1); // Continua até voltar ao início da lista
  i = (curr + 1) % mem_pool->pt.len;

  sem_post(&mem_pool->memory_s);
  return -1;
}
