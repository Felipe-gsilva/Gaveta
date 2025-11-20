#ifndef _UTILS
#define _UTILS

#include "../../defines.h"

void sleep_ms(u32 ms);

bool valid_int(int v);

u32 abs_diff(u32 a, u32 b);

char *sanitize_str(char *str);

#endif
