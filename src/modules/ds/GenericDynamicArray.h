// This is a old impl which does not follow the current structure
// I am going to use it anyway for simplicity
#ifndef __dynam_arr
#define __dynam_arr

#include "../../defines.h"

typedef struct DynamicArray {
  void *data;
  u32 size;
  u32 capacity;
  u32 element_size;
} DynamicArray;

DynamicArray *darray_create(u32 element_size, u32 initial_capacity);
void darray_destroy(DynamicArray *array);
int darray_push(DynamicArray *array, const void *element);
int darray_pop(DynamicArray *array, void *out_element);
int darray_get(const DynamicArray *array, size_t index, void *out_element);
int darray_set(DynamicArray *array, size_t index, const void *element);
u32 darray_size(const DynamicArray *array);
u32 darray_capacity(const DynamicArray *array);
int darray_get_last(const DynamicArray *array, void *out_element);
bool export_da_to_disk(const DynamicArray *array);

#endif
