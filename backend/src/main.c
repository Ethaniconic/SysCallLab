#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "../include/common.h"
#include "../include/config.h"
#include "../include/platform.h"
#include "../include/http_server.h"
#include "../include/api_routes.h"
#include "../include/database.h"
#include "../include/logger.h"

static void handle_sigint(int sig) {
    (void)sig;
    printf("\n[SysCallLab] Shutting down gracefully...\n");
    http_server_stop();
}

int main(int argc, char* argv[]) {
    printf("====================================================\n");
    printf("  %s v%s\n", SYSCALL_APP_NAME, SYSCALL_APP_VERSION);
    printf("  %s\n", SYSCALL_APP_SUBTITLE);
    printf("  Host Platform: %s\n", PLATFORM_NAME);
    printf("====================================================\n\n");

    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);

    config_init_defaults();
    if (argc > 1) {
        g_app_config.port = atoi(argv[1]);
    }

    config_ensure_directories();
    logger_init();

    logger_log(LOG_LEVEL_INFO, "Initializing platform subsystem...");
    if (!platform_init()) {
        logger_log(LOG_LEVEL_ERROR, "Platform initialization failed!");
        return 1;
    }

    logger_log(LOG_LEVEL_INFO, "Connecting persistent database (%s)...", g_app_config.db_path);
    if (!database_init(g_app_config.db_path)) {
        logger_log(LOG_LEVEL_ERROR, "Database initialization failed!");
        platform_cleanup();
        return 1;
    }

    logger_log(LOG_LEVEL_INFO, "Registering HTTP endpoints...");
    api_routes_register_all();

    logger_log(LOG_LEVEL_INFO, "Starting SysCallLab Local C Server on http://%s:%d", g_app_config.host, g_app_config.port);
    if (!http_server_init(g_app_config.host, g_app_config.port)) {
        logger_log(LOG_LEVEL_ERROR, "Failed to start HTTP server!");
        database_close();
        platform_cleanup();
        return 1;
    }

    printf("\n>>> SysCallLab is running offline at: http://127.0.0.1:%d <<<\n\n", g_app_config.port);
    http_server_start();

    database_close();
    logger_close();
    platform_cleanup();
    printf("[SysCallLab] Exited cleanly.\n");
    return 0;
}
