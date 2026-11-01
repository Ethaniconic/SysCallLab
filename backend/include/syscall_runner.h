#ifndef SYSCALL_RUNNER_H
#define SYSCALL_RUNNER_H

#include "common.h"

// Core file execution
ExecutionResult syscall_execute_file_open(const char* filepath, const char* mode);
ExecutionResult syscall_execute_file_read(const char* filepath, size_t bytes_to_read);
ExecutionResult syscall_execute_file_write(const char* filepath, const char* content);
ExecutionResult syscall_execute_file_close_invalid(int64_t fake_handle);
ExecutionResult syscall_execute_file_missing_test(const char* custom_path);
ExecutionResult syscall_execute_file_readonly_write_test(const char* filepath);

// Core process execution
ExecutionResult syscall_execute_process(const char* exec_path, const char* args, uint32_t timeout_ms);
ExecutionResult syscall_execute_missing_process_test(const char* custom_path);

#endif // SYSCALL_RUNNER_H
