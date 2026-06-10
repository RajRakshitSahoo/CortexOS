#ifndef LOGS_H
#define LOGS_H

#include "common.h"

typedef enum {
    LOG_INFO  = 0,
    LOG_WARN  = 1,
    LOG_ERROR = 2,
    LOG_AUTH  = 3,
    LOG_FS    = 4,
    LOG_PROC  = 5
} LogLevel;

void logs_init(void);
void log_write(LogLevel level, const char *message);
void logs_print(int last_n);          /* 0 = all */

void cmd_logs(int argc, char **argv);

#endif
