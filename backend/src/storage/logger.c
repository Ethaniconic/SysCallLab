#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "../../include/logger.h"

static FILE* s_log_file = NULL;

void logger_init(void) {
    if (!s_log_file) {
        s_log_file = fopen("data/syscalllab.log", "a");
    }
}

void logger_log(LogLevel level, const char* format, ...) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[32] = {0};
    if (tm_info) {
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        snprintf(time_str, sizeof(time_str), "UNKNOWN_TIME");
    }

    const char* lvl_str = "INFO";
    switch (level) {
        case LOG_LEVEL_WARN: lvl_str = "WARN"; break;
        case LOG_LEVEL_ERROR: lvl_str = "ERROR"; break;
        case LOG_LEVEL_DEBUG: lvl_str = "DEBUG"; break;
        case LOG_LEVEL_INFO:
        default: lvl_str = "INFO"; break;
    }

    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    printf("[%s] [%s] %s\n", time_str, lvl_str, message);

    if (s_log_file) {
        fprintf(s_log_file, "[%s] [%s] %s\n", time_str, lvl_str, message);
        fflush(s_log_file);
    }
}

void logger_close(void) {
    if (s_log_file) {
        fclose(s_log_file);
        s_log_file = NULL;
    }
}
