#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/json_handler.h"

cJSON* json_execution_result_to_json(const ExecutionResult* res) {
    if (!res) return NULL;
    cJSON* root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "success", res->success);
    cJSON_AddStringToObject(root, "execution_id", res->execution_id);
    cJSON_AddStringToObject(root, "timestamp", res->timestamp);
    cJSON_AddStringToObject(root, "operation", res->operation);
    cJSON_AddStringToObject(root, "platform", PLATFORM_NAME);
    cJSON_AddStringToObject(root, "native_api", res->native_api);
    cJSON_AddNumberToObject(root, "return_value", (double)res->return_value);
    cJSON_AddNumberToObject(root, "native_error_code", res->native_error_code);
    cJSON_AddStringToObject(root, "error_identifier", res->error_identifier);
    cJSON_AddStringToObject(root, "error_category", res->error_category_str);
    cJSON_AddStringToObject(root, "description", res->description);
    cJSON_AddStringToObject(root, "possible_cause", res->possible_cause);
    cJSON_AddStringToObject(root, "suggested_recovery", res->suggested_recovery);
    cJSON_AddStringToObject(root, "target_path", res->target_path);
    cJSON_AddStringToObject(root, "recovery_status", recovery_status_to_string(res->recovery_status));
    cJSON_AddStringToObject(root, "recovery_notes", res->recovery_notes);
    cJSON_AddNumberToObject(root, "elapsed_us", (double)res->elapsed_us);
    return root;
}

bool json_parse_file_execute_request(const char* json_str, char* path, char* operation, char* mode, char* content, size_t* read_size) {
    if (!json_str) return false;
    cJSON* root = cJSON_Parse(json_str);
    if (!root) return false;

    cJSON* p = cJSON_GetObjectItem(root, "path");
    if (cJSON_IsString(p) && path) {
        strncpy(path, p->valuestring, MAX_PATH_LENGTH - 1);
    }

    cJSON* op = cJSON_GetObjectItem(root, "operation");
    if (cJSON_IsString(op) && operation) {
        strncpy(operation, op->valuestring, 63);
    }

    cJSON* m = cJSON_GetObjectItem(root, "mode");
    if (cJSON_IsString(m) && mode) {
        strncpy(mode, m->valuestring, 15);
    }

    cJSON* c = cJSON_GetObjectItem(root, "content");
    if (cJSON_IsString(c) && content) {
        strncpy(content, c->valuestring, MAX_BUFFER_SIZE - 1);
    }

    cJSON* sz = cJSON_GetObjectItem(root, "read_size");
    if (cJSON_IsNumber(sz) && read_size) {
        *read_size = (size_t)sz->valueint;
    }

    cJSON_Delete(root);
    return true;
}

bool json_parse_process_execute_request(const char* json_str, char* exec_path, char* arguments, uint32_t* timeout_ms) {
    if (!json_str) return false;
    cJSON* root = cJSON_Parse(json_str);
    if (!root) return false;

    cJSON* ep = cJSON_GetObjectItem(root, "executable_path");
    if (cJSON_IsString(ep) && exec_path) {
        strncpy(exec_path, ep->valuestring, MAX_PATH_LENGTH - 1);
    }

    cJSON* args = cJSON_GetObjectItem(root, "arguments");
    if (cJSON_IsString(args) && arguments) {
        strncpy(arguments, args->valuestring, MAX_BUFFER_SIZE - 1);
    }

    cJSON* t = cJSON_GetObjectItem(root, "timeout_ms");
    if (cJSON_IsNumber(t) && timeout_ms) {
        *timeout_ms = (uint32_t)t->valueint;
    }

    cJSON_Delete(root);
    return true;
}

bool json_parse_recovery_request(const char* json_str, char* execution_id, char* action, bool* user_confirmed) {
    if (!json_str) return false;
    cJSON* root = cJSON_Parse(json_str);
    if (!root) return false;

    cJSON* id = cJSON_GetObjectItem(root, "execution_id");
    if (cJSON_IsString(id) && execution_id) {
        strncpy(execution_id, id->valuestring, MAX_ID_LENGTH - 1);
    }

    cJSON* act = cJSON_GetObjectItem(root, "action");
    if (cJSON_IsString(act) && action) {
        strncpy(action, act->valuestring, 63);
    }

    cJSON* conf = cJSON_GetObjectItem(root, "user_confirmed");
    if (cJSON_IsBool(conf) && user_confirmed) {
        *user_confirmed = cJSON_IsTrue(conf);
    }

    cJSON_Delete(root);
    return true;
}
