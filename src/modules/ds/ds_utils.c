#include "ds_utils.h"

void print_int_node(void *data) {
  assert((int*)data);
  printf("%d\n", *(int*)data);
}

void print_float_node(void *data) {
  assert((float*)data);
  printf("%f\n", *(float*)data);
}

void print_string_node(void *data) {
  assert((char*)data);
  printf("%s\n", (char*)data);
}


bool compare_ints(void *v1, void *v2) {
  assert(v1 && v2);
  printf("Comparing %d and %d\n", *(int*)v1, *(int*)v2);
  int *i1 = v1;
  int *i2 = v2;
  return *i2 == *i1;
}

bool compare_floats(void *v1, void *v2) {
  assert(v1 && v2);
  float *f1 = v1;
  float *f2 = v2;
  return *f2 == *f1;
}

bool compare_strings(void *v1, void *v2) {
  assert(v1 && v2);
  char *s1 = v1;
  char *s2 = v2;
  return strcmp(s1, s2) == 0;
}


void write_int_node(FILE *fp, void *data) {
  assert((int*)data && fp);
  fprintf(fp, "%d\n", *(int*)data);
}

void write_float_node(FILE *fp, void *data) {
  assert((int*)data && fp);
  fprintf(fp, "%d\n", *(int*)data);
}

void write_string_node(FILE *fp, void *data) {
  assert((int*)data && fp);
  fprintf(fp, "%d\n", *(int*)data);
}
