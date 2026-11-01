#ifndef SYSCALL_LOGGER_H
#define SYSCALL_LOGGER_H

#include "common.h"

typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_DEBUG
} LogLevel;

void logger_init(void);
void logger_log(LogLevel level, const char* format, ...);
void logger_close(void);

#endif // SYSCALL_LOGGER_H
