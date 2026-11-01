#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/api_routes.h"
#include "../../include/json_handler.h"
#include "../../include/syscall_runner.h"
#include "../../include/error_analyzer.h"
#include "../../include/recovery.h"
#include "../../include/test_runner.h"
#include "../../include/database.h"
#include "../../include/platform.h"
#include "../../include/logger.h"
#include "../../include/config.h"

void handle_api_health(const HttpRequest* req, HttpResponse* res) {
    (void)req;
    cJSON* json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "status", true);
    cJSON_AddStringToObject(json, "service", "SysCallLab Backend");
    cJSON_AddStringToObject(json, "version", SYSCALL_APP_VERSION);
    cJSON_AddStringToObject(json, "platform", PLATFORM_NAME);
    char* rendered = cJSON_PrintUnformatted(json);
    http_response_json(res, 200, rendered);
    free(rendered);
    cJSON_Delete(json);
}

void handle_api_system_info(const HttpRequest* req, HttpResponse* res) {
    (void)req;
    SystemInfo info;
    platform_get_system_info(&info);

    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "os_name", info.os_name);
    cJSON_AddStringToObject(json, "os_version", info.os_version);
    cJSON_AddStringToObject(json, "architecture", info.architecture);
    cJSON_AddStringToObject(json, "compiler_info", info.compiler_info);
    cJSON_AddStringToObject(json, "hostname", info.hostname);
    cJSON_AddNumberToObject(json, "process_id", info.process_id);
    cJSON_AddNumberToObject(json, "thread_id", info.thread_id);
    cJSON_AddNumberToObject(json, "total_ram_mb", (double)info.total_ram_mb);
    cJSON_AddNumberToObject(json, "free_ram_mb", (double)info.free_ram_mb);
    cJSON_AddNumberToObject(json, "cpu_cores", info.cpu_cores);

    char* rendered = cJSON_PrintUnformatted(json);
    http_response_json(res, 200, rendered);
    free(rendered);
    cJSON_Delete(json);
}

void handle_api_system_compatibility(const HttpRequest* req, HttpResponse* res) {
    (void)req;
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "current_platform", PLATFORM_NAME);
    cJSON_AddBoolToObject(json, "file_operations_supported", true);
    cJSON_AddBoolToObject(json, "process_operations_supported", true);
    cJSON_AddBoolToObject(json, "error_recovery_supported", true);
    cJSON_AddBoolToObject(json, "sqlite_persistence_supported", true);

    char* rendered = cJSON_PrintUnformatted(json);
    http_response_json(res, 200, rendered);
    free(rendered);
    cJSON_Delete(json);
}

void handle_api_statistics(const HttpRequest* req, HttpResponse* res) {
    (void)req;
    OperationalStats stats;
    database_get_stats(&stats);

    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "total_executions", stats.total_executions);
    cJSON_AddNumberToObject(json, "successful_executions", stats.successful_executions);
    cJSON_AddNumberToObject(json, "failed_executions", stats.failed_executions);
    cJSON_AddNumberToObject(json, "recovered_operations", stats.recovered_operations);

    char* rendered = cJSON_PrintUnformatted(json);
    http_response_json(res, 200, rendered);
    free(rendered);
    cJSON_Delete(json);
}

void handle_api_file_execute(const HttpRequest* req, HttpResponse* res) {
    char path[MAX_PATH_LENGTH] = {0};
    char operation[64] = {0};
    char mode[16] = "r";
    char content[MAX_BUFFER_SIZE] = {0};
    size_t read_size = 512;

    json_parse_file_execute_request(req->body, path, operation, mode, content, &read_size);

    ExecutionResult result;
    memset(&result, 0, sizeof(result));

    if (strcmp(operation, "open") == 0) {
        result = syscall_execute_file_open(path, mode);
    } else if (strcmp(operation, "read") == 0) {
        result = syscall_execute_file_read(path, read_size);
    } else if (strcmp(operation, "write") == 0) {
        result = syscall_execute_file_write(path, content);
    } else if (strcmp(operation, "missing_test") == 0) {
        result = syscall_execute_file_missing_test(path);
    } else if (strcmp(operation, "invalid_descriptor") == 0) {
        result = syscall_execute_file_close_invalid((int64_t)0xDEADBEEF);
    } else if (strcmp(operation, "readonly_test") == 0) {
        result = syscall_execute_file_readonly_write_test(path);
    } else {
        http_response_error(res, 400, "Unknown file operation requested");
        return;
    }

    database_insert_execution(&result);

    cJSON* json = json_execution_result_to_json(&result);
    char* rendered = cJSON_PrintUnformatted(json);
    http_response_json(res, 200, rendered);
    free(rendered);
    cJSON_Delete(json);
}

void handle_api_process_execute(const HttpRequest* req, HttpResponse* res) {
    char exec_path[MAX_PATH_LENGTH] = {0};
    char arguments[MAX_BUFFER_SIZE] = {0};
    uint32_t timeout_ms = 5000;

    json_parse_process_execute_request(req->body, exec_path, arguments, &timeout_ms);

    ExecutionResult result;
    if (strcmp(exec_path, "missing_test") == 0) {
        result = syscall_execute_missing_process_test(NULL);
    } else {
        result = syscall_execute_process(exec_path, arguments, timeout_ms);
    }

    database_insert_execution(&result);

    cJSON* json = json_execution_result_to_json(&result);
    char* rendered = cJSON_PrintUnformatted(json);
    http_response_json(res, 200, rendered);
    free(rendered);
    cJSON_Delete(json);
}

void handle_api_error_analyze(const HttpRequest* req, HttpResponse* res) {
    char execution_id[MAX_ID_LENGTH] = {0};
    cJSON* root = cJSON_Parse(req->body);
    if (root) {
        cJSON* item = cJSON_GetObjectItem(root, "execution_id");
        if (cJSON_IsString(item)) {
            strncpy(execution_id, item->valuestring, sizeof(execution_id) - 1);
        }
        cJSON_Delete(root);
    }

    if (strlen(execution_id) == 0) {
        http_response_error(res, 400, "Missing execution_id in analyze request");
        return;
    }

    ExecutionResult stored;
    if (!database_get_execution_by_id(execution_id, &stored)) {
        http_response_error(res, 404, "Execution ID not found in database");
        return;
    }

    // Re-enrich with current error analyzer logic
    error_analyzer_enrich(&stored);

    cJSON* json = json_execution_result_to_json(&stored);
    char* rendered = cJSON_PrintUnformatted(json);
    http_response_json(res, 200, rendered);
    free(rendered);
    cJSON_Delete(json);
}

void handle_api_recovery_execute(const HttpRequest* req, HttpResponse* res) {
    char execution_id[MAX_ID_LENGTH] = {0};
    char action[64] = "auto_heal";
    bool user_confirmed = false;

    json_parse_recovery_request(req->body, execution_id, action, &user_confirmed);

    RecoveryExecutionResult rec_res;
    bool ok = recovery_execute(execution_id, action, user_confirmed, &rec_res);

    cJSON* json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "success", ok);
    cJSON_AddStringToObject(json, "execution_id", rec_res.execution_id);
    cJSON_AddStringToObject(json, "strategy", rec_res.strategy_name);
    cJSON_AddBoolToObject(json, "user_confirmed", rec_res.user_confirmed);
    cJSON_AddStringToObject(json, "details", rec_res.details);
    cJSON_AddStringToObject(json, "recovery_status", ok ? "RECOVERED" : "RECOVERY_FAILED");

    char* rendered = cJSON_PrintUnformatted(json);
    http_response_json(res, 200, rendered);
    free(rendered);
    cJSON_Delete(json);
}

void handle_api_tests_run(const HttpRequest* req, HttpResponse* res) {
    char category[64] = "all";
    cJSON* root = cJSON_Parse(req->body);
    if (root) {
        cJSON* cat_item = cJSON_GetObjectItem(root, "category");
        if (cJSON_IsString(cat_item)) {
            strncpy(category, cat_item->valuestring, sizeof(category) - 1);
        }
        cJSON_Delete(root);
    }

    TestSuiteSummary summary;
    test_runner_run_suite(category, &summary);

    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "total_tests", summary.total_tests);
    cJSON_AddNumberToObject(json, "passed_tests", summary.passed_tests);
    cJSON_AddNumberToObject(json, "failed_tests", summary.failed_tests);

    cJSON* arr = cJSON_CreateArray();
    for (int i = 0; i < summary.total_tests; i++) {
        cJSON* item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "test_id", summary.results[i].test_id);
        cJSON_AddStringToObject(item, "name", summary.results[i].name);
        cJSON_AddStringToObject(item, "category", summary.results[i].category);
        cJSON_AddStringToObject(item, "operation", summary.results[i].operation);
        cJSON_AddStringToObject(item, "expected_outcome", summary.results[i].expected_outcome);
        cJSON_AddStringToObject(item, "actual_outcome", summary.results[i].actual_outcome);
        cJSON_AddNumberToObject(item, "actual_error_code", summary.results[i].actual_error_code);
        cJSON_AddBoolToObject(item, "passed", summary.results[i].passed);
        cJSON_AddStringToObject(item, "details", summary.results[i].details);
        cJSON_AddItemToArray(arr, item);
    }
    cJSON_AddItemToObject(json, "tests", arr);

    char* rendered = cJSON_PrintUnformatted(json);
    http_response_json(res, 200, rendered);
    free(rendered);
    cJSON_Delete(json);
}

static void history_row_cb(const ExecutionResult* row, void* user_data) {
    cJSON* arr = (cJSON*)user_data;
    cJSON* obj = json_execution_result_to_json(row);
    cJSON_AddItemToArray(arr, obj);
}

void handle_api_history(const HttpRequest* req, HttpResponse* res) {
    (void)req;
    cJSON* root = cJSON_CreateObject();
    cJSON* arr = cJSON_CreateArray();

    database_query_executions(NULL, NULL, 50, 0, history_row_cb, arr);

    cJSON_AddItemToObject(root, "executions", arr);
    char* rendered = cJSON_PrintUnformatted(root);
    http_response_json(res, 200, rendered);
    free(rendered);
    cJSON_Delete(root);
}

void handle_api_history_detail(const HttpRequest* req, HttpResponse* res) {
    const char* id = strrchr(req->path, '/');
    if (id && strlen(id) > 1) {
        id++;
        ExecutionResult item;
        if (database_get_execution_by_id(id, &item)) {
            cJSON* json = json_execution_result_to_json(&item);
            char* rendered = cJSON_PrintUnformatted(json);
            http_response_json(res, 200, rendered);
            free(rendered);
            cJSON_Delete(json);
            return;
        }
    }
    http_response_error(res, 404, "Execution ID not found");
}

void handle_api_logs(const HttpRequest* req, HttpResponse* res) {
    // Audit logs share execution query
    handle_api_history(req, res);
}

void handle_api_logs_export_csv(const HttpRequest* req, HttpResponse* res) {
    (void)req;
    char export_path[MAX_PATH_LENGTH];
    snprintf(export_path, sizeof(export_path), "%s/audit_export_%lx.csv", g_app_config.temp_dir, (unsigned long)time(NULL));

    if (database_export_csv(export_path)) {
        http_response_file(res, export_path, "text/csv");
    } else {
        http_response_error(res, 500, "Failed to generate CSV export");
    }
}

void handle_api_settings(const HttpRequest* req, HttpResponse* res) {
    (void)req;
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "host", g_app_config.host);
    cJSON_AddNumberToObject(json, "port", g_app_config.port);
    cJSON_AddStringToObject(json, "db_path", g_app_config.db_path);
    cJSON_AddStringToObject(json, "temp_dir", g_app_config.temp_dir);
    cJSON_AddBoolToObject(json, "verbose_logging", g_app_config.verbose_logging);

    char* rendered = cJSON_PrintUnformatted(json);
    http_response_json(res, 200, rendered);
    free(rendered);
    cJSON_Delete(json);
}

void api_routes_register_all(void) {
    http_server_register_route("GET", "/api/health", handle_api_health);
    http_server_register_route("GET", "/api/system/info", handle_api_system_info);
    http_server_register_route("GET", "/api/system/compatibility", handle_api_system_compatibility);
    http_server_register_route("GET", "/api/statistics", handle_api_statistics);

    http_server_register_route("POST", "/api/file/execute", handle_api_file_execute);
    http_server_register_route("POST", "/api/process/execute", handle_api_process_execute);
    http_server_register_route("POST", "/api/error/analyze", handle_api_error_analyze);
    http_server_register_route("POST", "/api/recovery/execute", handle_api_recovery_execute);
    http_server_register_route("POST", "/api/tests/run", handle_api_tests_run);

    http_server_register_route("GET", "/api/history", handle_api_history);
    http_server_register_route("GET", "/api/logs", handle_api_logs);
    http_server_register_route("GET", "/api/logs/export", handle_api_logs_export_csv);
    http_server_register_route("GET", "/api/settings", handle_api_settings);
}
