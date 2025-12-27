#include "log.h"
#include "../utils/utils.h"
#include "stdio.h"
#include <time.h>

bool debug;
log_level min_log_level;

log_level get_min_log_level(void) {
  if (debug)
    return INFO;
  return WARN;
}

void g_log(log_level level, status_code status, const char *str, ...) {
  if (level < min_log_level) return;

  time_t clk = time(NULL);

  printf("%-10s", sanitize_str(ctime(&clk) + 11));
  switch (level) {
    case ERROR:
      printf("\033[31;1;4mERROR\033[0m ");
      break;
    case WARN:
      printf("\033[93;1;2mWARN\033[0m ");
      break;
    case INFO:
      printf("\033[1;94;1mINFO\033[0m ");
      break;
    case DEBUG:
      printf("\033[1;95;4mDEBUG\033[0m ");
      break;
    case CRIT_ERROR:
      printf("\033[31;1;4mCRIT_ERROR\033[0m ");
      break;
    default:
      break;
  }

  switch (status / 100) {
    case -1:
      break;
    case 0:
      printf("%d - App status | ", status);
      break;
    case 1: 
      printf("%d - Btree Status | ", status);
      break; 
    case 2:
      printf("%d - File Error | ", status);
      break;
    case 3:
      printf("%d - Memory Status | ", status);
      break;
    case 4: 
      printf("%d - Disk Status | ", status);
      break;
    case 5:
      printf("%d - User Status Context | ", status);
      break;
    case 6: 
      printf("%d - Invalid Input | ", status);
      break;
    case 7: 
      printf("%d - Internal DS Status | ", status);
      break;
    default:
      printf("%d - Unknown Status | ", status);
  }

  char buffer[4096];
  va_list arg_list;
  va_start(arg_list, str);
  vsnprintf(buffer, 255, str, arg_list);
  va_end(arg_list);
  printf("%s\n", sanitize_str(buffer));
}
