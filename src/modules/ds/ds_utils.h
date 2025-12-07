#ifndef _DS_UTILS_H
#define _DS_UTILS_H

#include "../../defines.h"
#include "../log/log.h"
#include "../memory/mem.h"


typedef void (*print_callback_fn)(void *data);
typedef void (*write_fallback_fn)(FILE *fp, void *data);
typedef void (*read_fallback_fn)(FILE *fp, void *data);

bool compare_ints(void *a, void *b);
bool compare_floats(void *a, void *b);
bool compare_strings(void *a, void *b);

void print_int_node(void *data);
void print_float_node(void *data);
void print_string_node(void *data);

void write_int_node(FILE *fp, void *data);
void write_float_node(FILE *fp, void *data);
void write_string_node(FILE *fp, void *data);

void read_int_node(FILE *fp, void *data);
void read_float_node(FILE *fp, void *data);
void read_string_node(FILE *fp, void *data);

#endif
