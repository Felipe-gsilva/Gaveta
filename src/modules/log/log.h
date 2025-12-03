#ifndef _LOG
#define _LOG

#include "../../defines.h"
#include <stdarg.h>

typedef enum { DEBUG, INFO, WARN, ERROR, CRIT_ERROR } log_level;

// this is just a todo status code, in order to make things easy to debug
// Im gonna make this prefix reffers to a system part
// yet to change
typedef enum {
  DEFAULT_STATUS = -100,
  APP_ERROR = 001,
  BTREE_STATUS = 100,
  BTREE_ERROR = 101,
  BTREE_CREATION_ERROR = 102,
  BTREE_DELETION_ERROR = 103,
  FILE_OPEN_ERROR = 200,
  MEM_STATUS = 300,
  MEM_ERROR = 301,
  MEM_FULL = 302,
  MEM_ALLOC_FAIL = 303,
  MEM_DEALLOC_FAIL = 304,
  MEM_REALLOC_FAIL = 305,
  DISK_STATUS = 400,
  DISK_ERROR = 401,
  DISK_FILE_READ_ERROR = 402,
  DISK_FILE_WRITE_ERROR = 403,
  DISK_OPEN_ERROR = 404,
  DISK_MISS = 405,
  SEMAPHORE_ERROR = 500,
  SEMAPHORE_INIT_ERROR = 501,
  SEMAPHORE_POST_ERROR = 502,
  SEMAPHORE_WAIT_ERROR = 503,
  THREAD_INIT_ERROR = 504,
  INVALID_INPUT = 600,
  INVALID_POINTER = 601,
  QUEUE_STATUS = 700,
  QUEUE_ERROR = 701,
  QUEUE_EMPTY = 702,
} status_code;

// defining correct macros for logging
#define g_crit_error(s, ...)                                                   \
  g_log(CRIT_ERROR, s, ##__VA_ARGS__, NULL);                                   \
  exit(1);
#define g_error(s, ...) g_log(ERROR, s, ##__VA_ARGS__, NULL)
#define g_warn(s, ...) g_log(WARN, s, ##__VA_ARGS__, NULL)
#define g_info(...) g_log(INFO, DEFAULT_STATUS, ##__VA_ARGS__, NULL)
#define g_debug(s, ...) g_log(DEBUG, s, ##__VA_ARGS__, NULL)

log_level get_min_log_level();

// do not use this directly unless you are sure you need to
void g_log(log_level level, status_code status_code, const char *str, ...);

#endif
