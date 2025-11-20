#include "mem.h"
#include "../log/log.h"
#include "../../App.h"

extern App app;

void init_mem(u32 mem_size) {
  if (mem_size >= 4 * MB) {
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
  app.mem = mem;

  for (u32 i = 0; i < mem->pt.len; i++) {
    mem->pt.pages[i].id = i;
    mem->pt.pages[i].p = memory_pool + (i * PAGE_SIZE);
    mem->pt.pages[i].free = true;
    mem->pt.pages[i].used = false;
    push_free_stack(i);
  }

  if (!mem->pt.pages) {
    g_crit_error(MEM_ERROR, "Could not allocate pages!");
  }

  if (sem_init(&app.mem->memory_s, 0, 1) != 0) {
    g_crit_error(SEMAPHORE_INIT_ERROR, "Memory semaphore failed to initialize")
  }

  g_info("%dB allocated", (int)mem_size);
}

void clear_mem() {
  if (!app.mem) {
    g_crit_error(MEM_ERROR, "There is no memory to be freed");
  }

  int mem_size = app.mem->size;

  free(app.mem->pt.pages);
  free(app.mem->pt.free_stack);
  free(app.mem->pool);
  free(app.mem);

  g_info("%dB deallocated", mem_size);
}

void *g_alloc(u32 bytes) {
  g_info("Allocating %d bytes", bytes);
  if (bytes == 0)
    return NULL;

  u32 num_pages = (bytes + sizeof(alloc_header) + PAGE_SIZE - 1) / PAGE_SIZE;

  if (bytes >= app.mem->size) {
    g_error(MEM_ALLOC_FAIL,
            "greater chunk of memory than physical memory holds");
    return NULL;
  }

  g_info("Pages to be allocated: %d | Free pages: %d", num_pages,
         app.mem->pt.free_page_num);
  if (num_pages > app.mem->pt.free_page_num || num_pages > app.mem->pt.len) {
    g_info("Not enough memory to allocate %d pages", num_pages);
    while (num_pages > app.mem->pt.free_page_num) {
      int not_used_page = second_chance();
      g_info("Not used page %d", not_used_page);
      if (not_used_page == -1) {
        g_error(MEM_FULL, "Memory full even after page replacement!");
        sem_post(&app.mem->memory_s);
        return NULL;
      }

      sem_wait(&app.mem->memory_s);
      app.mem->pt.pages[not_used_page].free = true;
      app.mem->pt.free_page_num++;
      sem_post(&app.mem->memory_s);
      push_free_stack(not_used_page);
    }
  }

  sem_wait(&app.mem->memory_s);
  void *ptr = NULL;
  for (u32 i = 0; i < app.mem->pt.len; i++) {
    bool contiguos_region = true;
    if (i + num_pages > app.mem->pt.len) {
      break;
    }
    for (u32 j = i; j < i + num_pages; j++) {
      if (!app.mem->pt.pages[j].free) {
        contiguos_region = false;
        break;
      }
    }

    if (!contiguos_region)
      continue;
    g_info("Found %d contiguous pages at range %d - %d", num_pages, i,
           num_pages + i);
    ptr = (void*)((char*)app.mem->pool + (i * PAGE_SIZE));
    app.mem->pt.free_page_num -= num_pages;
    for (u32 j = i; j < i + num_pages; j++)
    {
      app.mem->pt.pages[j].free = false;
      app.mem->pt.pages[j].used = true;
    }
    alloc_header* h_ptr = ptr;
    h_ptr->id = i;
    h_ptr->page_num = num_pages;
    break;
  }

  if (!ptr) {
    sem_post(&app.mem->memory_s);
    g_crit_error(MEM_ALLOC_FAIL,
                 "Failed to allocate %d bytes of memory with %d pages", bytes,
                 num_pages);
    return NULL;
  }

  sem_post(&app.mem->memory_s);
  g_debug(MEM_STATUS, "Allocated %d pages for %d bytes", num_pages, bytes);
  return (void *)(char *)ptr + sizeof(alloc_header);
}

void *g_realloc(void *curr_region, u32 bytes) {
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

  void *buffer = g_alloc(total_size);
  if (!buffer) {
    g_error(MEM_REALLOC_FAIL, "failed to realloc %d + %d bytes", old_size,
            bytes);
    return NULL;
  }

  memcpy(buffer, curr_region, old_size);
  c_dealloc(curr_region);

  g_info("region %p reallocated %d to %d bytes", buffer, old_size, bytes);

  return buffer;
}
void push_free_stack(u32 i) {
  if (app.mem->pt.free_stack_top < app.mem->pt.len) {
    app.mem->pt.free_stack[app.mem->pt.free_stack_top++] = i;
  }
}

void g_dealloc(void *mem) {
  sem_wait(&app.mem->memory_s);
  if (!mem) {
    g_error(MEM_DEALLOC_FAIL,
            "Trying to deallocate a non allocated memory region");
    sem_post(&app.mem->memory_s);
    return;
  }

  alloc_header *h_ptr = get_header(mem);

  for (u32 i = h_ptr->id; i < h_ptr->id + h_ptr->page_num; i++) {
    app.mem->pt.pages[i].free = true;
    push_free_stack(i);
  }

  app.mem->pt.free_page_num += h_ptr->page_num;
  sem_post(&app.mem->memory_s);
}

alloc_header *get_header(void *ptr) {
  return (alloc_header *)((char *)ptr - sizeof(alloc_header));
}

float retrieve_free_mem_percentage(void) {
  return (float)app.mem->pt.free_page_num / (float)app.mem->pt.len * 100.0f;
}

float retrieve_used_mem_percentage(void) {
  return 100.0f - retrieve_free_mem_percentage();
}
void print_page_table_status() {
  for (u32 i = 0; i < app.mem->pt.len; i++) {
    if (app.mem->pt.pages[i].p)
      g_info("%d %d", i, app.mem->pt.pages[i].id);
  }
}

bool is_mem_free(void *ptr) {
  if (!ptr)
    return true;

  sem_wait(&app.mem->memory_s);

  alloc_header *h_ptr = get_header(ptr);
  bool is_free = true;
  for (u32 i = 0; i < h_ptr->page_num; i++) {
    page *p = (page *)(char *)app.mem->pool + (i * PAGE_SIZE);
    if (!p->free) {
      is_free = false;
      break;
    }
  }
  sem_post(&app.mem->memory_s);
  return is_free;
}

int second_chance() {
  static int i = 0;
  int curr = i;

  sem_wait(&app.mem->memory_s);
  do {
    g_info("Page %d", curr);
    page *p = &app.mem->pt.pages[curr];
    if (!(p->used)) {
      p->used = false;
      sem_post(&app.mem->memory_s);
      return (i = (curr + 1) % app.mem->pt.len);
    }
    p->used = false;
    curr = (curr + 1) %
           app.mem->pt.len; // Atualiza o valor da curr para a próxima página
  } while (curr != i - 1); // Continua até voltar ao início da lista
  i = (curr + 1) % app.mem->pt.len;

  sem_post(&app.mem->memory_s);
  return -1;
}
