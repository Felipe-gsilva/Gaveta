#include "utils.h"
#include "assert.h"
#include "stdint.h"
#include <unistd.h>

void sleep_ms(u32 ms) { usleep(ms * 1000); }

bool valid_int(int v) { return (v > 0 && v < INT32_MAX) ? true : false; }

u32 abs_diff(u32 a, u32 b) { return a > b ? a - b : b - a; }

char *sanitize_str(char *str) {
  if (!str) {
    return NULL;
  }
  for (size_t i = 0; i < strlen(str); i++) {
    if (str[i] == '\n' || str[i] == '-') {
      str[i] = ' ';
    }
  }

  return str;
}
