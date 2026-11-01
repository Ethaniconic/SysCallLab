#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/http_server.h"
#include "../../include/logger.h"
#include "../../include/config.h"

#if defined(PLATFORM_WINDOWS)
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define CLOSE_SOCKET(s) closesocket(s)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int socket_t;
#define CLOSE_SOCKET(s) close(s)
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

static socket_t s_server_socket = INVALID_SOCKET;
static bool s_running = false;
static RouteEntry s_routes[64];
static int s_route_count = 0;

void http_server_register_route(const char* method, const char* path, RouteHandler handler) {
    if (s_route_count < 64) {
        strncpy(s_routes[s_route_count].method, method, 15);
        strncpy(s_routes[s_route_count].path, path, 127);
        s_routes[s_route_count].handler = handler;
        s_route_count++;
    }
}

void http_response_json(HttpResponse* res, int status, const char* json_str) {
    if (!res) return;
    res->status_code = status;
    strncpy(res->content_type, "application/json; charset=utf-8", sizeof(res->content_type) - 1);
    if (json_str) {
        res->body_len = strlen(json_str);
        res->body = strdup(json_str);
        res->body_dynamic = true;
    } else {
        res->body = NULL;
        res->body_len = 0;
        res->body_dynamic = false;
    }
}

void http_response_text(HttpResponse* res, int status, const char* text) {
    if (!res) return;
    res->status_code = status;
    strncpy(res->content_type, "text/plain; charset=utf-8", sizeof(res->content_type) - 1);
    if (text) {
        res->body_len = strlen(text);
        res->body = strdup(text);
        res->body_dynamic = true;
    }
}

void http_response_error(HttpResponse* res, int status, const char* message) {
    char buf[512];
    snprintf(buf, sizeof(buf), "{\"success\":false,\"error\":\"%s\"}", message ? message : "Internal Error");
    http_response_json(res, status, buf);
}

void http_response_file(HttpResponse* res, const char* filepath, const char* content_type) {
    if (!res) return;
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        http_response_error(res, 404, "File not found");
        return;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (sz < 0 || sz > 10 * 1024 * 1024) { // 10MB limit
        fclose(f);
        http_response_error(res, 500, "File too large");
        return;
    }

    char* data = malloc(sz + 1);
    if (!data) {
        fclose(f);
        http_response_error(res, 500, "Memory allocation error");
        return;
    }

    fread(data, 1, sz, f);
    data[sz] = '\0';
    fclose(f);

    res->status_code = 200;
    strncpy(res->content_type, content_type ? content_type : "text/html", sizeof(res->content_type) - 1);
    res->body = data;
    res->body_len = sz;
    res->body_dynamic = true;
}

static const char* get_content_type_from_path(const char* path) {
    const char* dot = strrchr(path, '.');
    if (!dot) return "text/plain";
    if (strcmp(dot, ".html") == 0) return "text/html";
    if (strcmp(dot, ".css") == 0) return "text/css";
    if (strcmp(dot, ".js") == 0) return "application/javascript";
    if (strcmp(dot, ".json") == 0) return "application/json";
    if (strcmp(dot, ".png") == 0) return "image/png";
    if (strcmp(dot, ".svg") == 0) return "image/svg+xml";
    if (strcmp(dot, ".ico") == 0) return "image/x-icon";
    return "text/plain";
}

static void serve_static_file(const HttpRequest* req, HttpResponse* res) {
    char clean_path[MAX_PATH_LENGTH];
    const char* p = req->path;
    if (strcmp(p, "/") == 0) {
        p = "/index.html";
    }

    // Path traversal check
    if (strstr(p, "..") != NULL) {
        http_response_error(res, 403, "Path traversal forbidden");
        return;
    }

    snprintf(clean_path, sizeof(clean_path), "%s%s", g_app_config.frontend_dir, p);
    http_response_file(res, clean_path, get_content_type_from_path(clean_path));
}

static void handle_client(socket_t client_sock) {
    char raw_req[MAX_BUFFER_SIZE * 8];
    int bytes = recv(client_sock, raw_req, sizeof(raw_req) - 1, 0);
    if (bytes <= 0) {
        CLOSE_SOCKET(client_sock);
        return;
    }
    raw_req[bytes] = '\0';

    HttpRequest req;
    memset(&req, 0, sizeof(req));
    HttpResponse res;
    memset(&res, 0, sizeof(res));

    // Find end of headers in raw request
    char* body_separator = strstr(raw_req, "\r\n\r\n");
    int header_skip = 4;
    if (!body_separator) {
        body_separator = strstr(raw_req, "\n\n");
        header_skip = 2;
    }

    // Find Content-Length
    int content_len = 0;
    char* cl_header = strstr(raw_req, "Content-Length:");
    if (!cl_header) cl_header = strstr(raw_req, "content-length:");
    if (cl_header) {
        content_len = atoi(cl_header + 15);
    }

    if (body_separator) {
        char* body_ptr = body_separator + header_skip;
        int current_body_len = bytes - (int)(body_ptr - raw_req);
        // If content_len > current_body_len, read remainder from socket
        while (current_body_len < content_len && bytes < (int)sizeof(raw_req) - 1) {
            int more = recv(client_sock, raw_req + bytes, sizeof(raw_req) - 1 - bytes, 0);
            if (more <= 0) break;
            bytes += more;
            current_body_len += more;
            raw_req[bytes] = '\0';
        }
        size_t blen = (size_t)current_body_len;
        if (blen >= sizeof(req.body)) blen = sizeof(req.body) - 1;
        memcpy(req.body, body_ptr, blen);
        req.body[blen] = '\0';
        req.body_len = blen;
    }

    // Parse method and path from first line
    char line_buf[512] = {0};
    char* eol = strstr(raw_req, "\r\n");
    if (!eol) eol = strchr(raw_req, '\n');
    if (eol) {
        size_t l = (size_t)(eol - raw_req);
        if (l >= sizeof(line_buf)) l = sizeof(line_buf) - 1;
        memcpy(line_buf, raw_req, l);
        line_buf[l] = '\0';
    } else {
        strncpy(line_buf, raw_req, sizeof(line_buf) - 1);
    }

    sscanf(line_buf, "%15s %255s", req.method, req.path);

    // Parse query string if present
    char* qmark = strchr(req.path, '?');
    if (qmark) {
        *qmark = '\0';
        strncpy(req.query, qmark + 1, sizeof(req.query) - 1);
    }

    // Handle OPTIONS for CORS
    if (strcmp(req.method, "OPTIONS") == 0) {
        const char* cors_hdr = 
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: 0\r\n\r\n";
        send(client_sock, cors_hdr, strlen(cors_hdr), 0);
        CLOSE_SOCKET(client_sock);
        return;
    }

    // Dispatch Route
    RouteHandler handler = NULL;
    for (int i = 0; i < s_route_count; i++) {
        if (strcmp(s_routes[i].method, req.method) == 0 && strcmp(s_routes[i].path, req.path) == 0) {
            handler = s_routes[i].handler;
            break;
        }
    }

    if (handler) {
        handler(&req, &res);
    } else {
        if (strcmp(req.method, "GET") == 0) {
            serve_static_file(&req, &res);
        } else {
            http_response_error(&res, 404, "Endpoint not found");
        }
    }

    // Construct HTTP response string
    char header_buf[1024];
    const char* status_text = "OK";
    if (res.status_code == 404) status_text = "Not Found";
    else if (res.status_code == 403) status_text = "Forbidden";
    else if (res.status_code == 400) status_text = "Bad Request";
    else if (res.status_code == 500) status_text = "Internal Server Error";

    int header_len = snprintf(header_buf, sizeof(header_buf),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n\r\n",
        res.status_code, status_text, res.content_type, res.body_len);

    send(client_sock, header_buf, header_len, 0);
    if (res.body && res.body_len > 0) {
        send(client_sock, res.body, (int)res.body_len, 0);
    }

    if (res.body_dynamic && res.body) {
        free(res.body);
    }

    CLOSE_SOCKET(client_sock);
}

bool http_server_init(const char* host, int port) {
    s_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (s_server_socket == INVALID_SOCKET) {
        logger_log(LOG_LEVEL_ERROR, "Failed to create socket");
        return false;
    }

    int opt = 1;
#if defined(PLATFORM_WINDOWS)
    setsockopt(s_server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(s_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(host ? host : "127.0.0.1");

    if (bind(s_server_socket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        logger_log(LOG_LEVEL_ERROR, "Failed to bind socket to %s:%d", host, port);
        CLOSE_SOCKET(s_server_socket);
        return false;
    }

    if (listen(s_server_socket, 16) == SOCKET_ERROR) {
        logger_log(LOG_LEVEL_ERROR, "Failed to listen on socket");
        CLOSE_SOCKET(s_server_socket);
        return false;
    }

    logger_log(LOG_LEVEL_INFO, "HTTP Server listening on http://%s:%d", host, port);
    return true;
}

void http_server_start(void) {
    s_running = true;
    while (s_running) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        socket_t client_sock = accept(s_server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock != INVALID_SOCKET) {
            handle_client(client_sock);
        }
    }
}

void http_server_stop(void) {
    s_running = false;
    if (s_server_socket != INVALID_SOCKET) {
        CLOSE_SOCKET(s_server_socket);
        s_server_socket = INVALID_SOCKET;
    }
}
