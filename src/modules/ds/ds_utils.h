#ifndef _DS_UTILS_H
#define _DS_UTILS_H

#include "../../defines.h"
#include "../log/log.h"
#include "../memory/mem.h"

bool compare_ints(void *a, void *b);
bool compare_floats(void *a, void *b);
bool compare_strings(void *a, void *b);

void print_int_node(void *data);
void print_float_node(void *data);
void print_string_node(void *data);

#endif
