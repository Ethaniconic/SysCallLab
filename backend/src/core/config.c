#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/config.h"
#include "../../include/platform.h"

#if defined(_WIN32)
#include <direct.h>
#define mkdir_compat(path) _mkdir(path)
#else
#include <sys/stat.h>
#define mkdir_compat(path) mkdir(path, 0755)
#endif

AppConfig g_app_config;

void config_init_defaults(void) {
    strncpy(g_app_config.host, SYSCALL_DEFAULT_HOST, sizeof(g_app_config.host) - 1);
    g_app_config.port = SYSCALL_DEFAULT_PORT;
    if (platform_file_exists("frontend/index.html")) {
        strncpy(g_app_config.frontend_dir, "frontend", sizeof(g_app_config.frontend_dir) - 1);
        strncpy(g_app_config.db_path, "data/syscalllab.db", sizeof(g_app_config.db_path) - 1);
        strncpy(g_app_config.temp_dir, "data/syscall_temp", sizeof(g_app_config.temp_dir) - 1);
        strncpy(g_app_config.helper_dir, "build/bin", sizeof(g_app_config.helper_dir) - 1);
    } else if (platform_file_exists("../frontend/index.html")) {
        strncpy(g_app_config.frontend_dir, "../frontend", sizeof(g_app_config.frontend_dir) - 1);
        strncpy(g_app_config.db_path, "../data/syscalllab.db", sizeof(g_app_config.db_path) - 1);
        strncpy(g_app_config.temp_dir, "../data/syscall_temp", sizeof(g_app_config.temp_dir) - 1);
        strncpy(g_app_config.helper_dir, "bin", sizeof(g_app_config.helper_dir) - 1);
    } else {
        strncpy(g_app_config.frontend_dir, "frontend", sizeof(g_app_config.frontend_dir) - 1);
        strncpy(g_app_config.db_path, "data/syscalllab.db", sizeof(g_app_config.db_path) - 1);
        strncpy(g_app_config.temp_dir, "data/syscall_temp", sizeof(g_app_config.temp_dir) - 1);
        strncpy(g_app_config.helper_dir, "build/bin", sizeof(g_app_config.helper_dir) - 1);
    }
    g_app_config.verbose_logging = true;
}

bool config_ensure_directories(void) {
    mkdir_compat("data");
    mkdir_compat(g_app_config.temp_dir);
    return true;
}
