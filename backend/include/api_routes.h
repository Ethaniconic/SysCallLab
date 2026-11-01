#ifndef SYSCALL_API_ROUTES_H
#define SYSCALL_API_ROUTES_H

#include "http_server.h"

void api_routes_register_all(void);

// Handlers
void handle_api_health(const HttpRequest* req, HttpResponse* res);
void handle_api_system_info(const HttpRequest* req, HttpResponse* res);
void handle_api_system_compatibility(const HttpRequest* req, HttpResponse* res);
void handle_api_statistics(const HttpRequest* req, HttpResponse* res);

void handle_api_file_execute(const HttpRequest* req, HttpResponse* res);
void handle_api_process_execute(const HttpRequest* req, HttpResponse* res);
void handle_api_error_analyze(const HttpRequest* req, HttpResponse* res);
void handle_api_recovery_execute(const HttpRequest* req, HttpResponse* res);
void handle_api_tests_run(const HttpRequest* req, HttpResponse* res);

void handle_api_history(const HttpRequest* req, HttpResponse* res);
void handle_api_history_detail(const HttpRequest* req, HttpResponse* res);
void handle_api_logs(const HttpRequest* req, HttpResponse* res);
void handle_api_logs_export_csv(const HttpRequest* req, HttpResponse* res);
void handle_api_settings(const HttpRequest* req, HttpResponse* res);

#endif // SYSCALL_API_ROUTES_H
