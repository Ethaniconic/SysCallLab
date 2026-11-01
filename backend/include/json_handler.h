#ifndef SYSCALL_JSON_HANDLER_H
#define SYSCALL_JSON_HANDLER_H

#include "common.h"
#include "../../third_party/cJSON/cJSON.h"

cJSON* json_execution_result_to_json(const ExecutionResult* result);
bool json_parse_file_execute_request(const char* json_str, char* path, char* operation, char* mode, char* content, size_t* read_size);
bool json_parse_process_execute_request(const char* json_str, char* exec_path, char* arguments, uint32_t* timeout_ms);
bool json_parse_recovery_request(const char* json_str, char* execution_id, char* action, bool* user_confirmed);

#endif // SYSCALL_JSON_HANDLER_H
