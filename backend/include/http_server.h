#ifndef SYSCALL_HTTP_SERVER_H
#define SYSCALL_HTTP_SERVER_H

#include "common.h"

typedef struct {
    char method[16];
    char path[256];
    char query[256];
    char body[MAX_BUFFER_SIZE * 4];
    size_t body_len;
    char client_ip[64];
} HttpRequest;

typedef struct {
    int status_code;
    char content_type[64];
    char* body;
    size_t body_len;
    bool body_dynamic;
} HttpResponse;

typedef void (*RouteHandler)(const HttpRequest* req, HttpResponse* res);

typedef struct {
    char method[16];
    char path[128];
    RouteHandler handler;
} RouteEntry;

bool http_server_init(const char* host, int port);
void http_server_register_route(const char* method, const char* path, RouteHandler handler);
void http_server_start(void);
void http_server_stop(void);

void http_response_json(HttpResponse* res, int status, const char* json_str);
void http_response_text(HttpResponse* res, int status, const char* text);
void http_response_file(HttpResponse* res, const char* filepath, const char* content_type);
void http_response_error(HttpResponse* res, int status, const char* message);

#endif // SYSCALL_HTTP_SERVER_H
