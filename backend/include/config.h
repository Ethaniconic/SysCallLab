#ifndef SYSCALL_CONFIG_H
#define SYSCALL_CONFIG_H

#include "common.h"

typedef struct {
    char host[64];
    int port;
    char frontend_dir[MAX_PATH_LENGTH];
    char db_path[MAX_PATH_LENGTH];
    char temp_dir[MAX_PATH_LENGTH];
    char helper_dir[MAX_PATH_LENGTH];
    bool verbose_logging;
} AppConfig;

extern AppConfig g_app_config;

void config_init_defaults(void);
bool config_ensure_directories(void);

#endif // SYSCALL_CONFIG_H
