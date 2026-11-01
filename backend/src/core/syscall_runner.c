#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/syscall_runner.h"
#include "../../include/error_analyzer.h"
#include "../../include/platform.h"
#include "../../include/config.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#endif

extern void time_get_iso8601(char* out_buf, size_t max_len);
extern void id_generate(const char* prefix, char* out_buf, size_t max_len);

ExecutionResult syscall_execute_file_open(const char* filepath, const char* mode) {
    ExecutionResult res;
    memset(&res, 0, sizeof(res));
    id_generate("f-open", res.execution_id, sizeof(res.execution_id));
    time_get_iso8601(res.timestamp, sizeof(res.timestamp));
    strncpy(res.operation, "file_open", sizeof(res.operation) - 1);
#if defined(PLATFORM_WINDOWS)
    strncpy(res.native_api, "CreateFileA", sizeof(res.native_api) - 1);
#else
    strncpy(res.native_api, "open", sizeof(res.native_api) - 1);
#endif
    strncpy(res.target_path, filepath ? filepath : "", sizeof(res.target_path) - 1);

    int64_t t0 = platform_get_time_us();
    int32_t err = 0;
    int64_t handle = platform_open_file(filepath, mode, &err);
    int64_t t1 = platform_get_time_us();

    res.elapsed_us = t1 - t0;
    res.return_value = handle;
    res.native_error_code = err;
    res.success = (handle != -1);
    res.recovery_status = RECOVERY_NOT_ATTEMPTED;

    if (res.success) {
        platform_close_file(handle, NULL);
    }

    error_analyzer_enrich(&res);
    return res;
}

ExecutionResult syscall_execute_file_read(const char* filepath, size_t bytes_to_read) {
    ExecutionResult res;
    memset(&res, 0, sizeof(res));
    id_generate("f-read", res.execution_id, sizeof(res.execution_id));
    time_get_iso8601(res.timestamp, sizeof(res.timestamp));
    strncpy(res.operation, "file_read", sizeof(res.operation) - 1);
#if defined(PLATFORM_WINDOWS)
    strncpy(res.native_api, "ReadFile", sizeof(res.native_api) - 1);
#else
    strncpy(res.native_api, "read", sizeof(res.native_api) - 1);
#endif
    strncpy(res.target_path, filepath ? filepath : "", sizeof(res.target_path) - 1);

    int32_t err = 0;
    int64_t handle = platform_open_file(filepath, "r", &err);
    if (handle == -1) {
        res.return_value = -1;
        res.native_error_code = err;
        res.success = false;
        error_analyzer_enrich(&res);
        return res;
    }

    if (bytes_to_read == 0 || bytes_to_read > MAX_BUFFER_SIZE) {
        bytes_to_read = 512;
    }

    char* buf = malloc(bytes_to_read + 1);
    int64_t t0 = platform_get_time_us();
    int64_t bytes_read = platform_read_file(handle, buf, bytes_to_read, &err);
    int64_t t1 = platform_get_time_us();
    platform_close_file(handle, NULL);

    res.elapsed_us = t1 - t0;
    res.return_value = bytes_read;
    res.native_error_code = err;
    res.success = (bytes_read >= 0);
    res.recovery_status = RECOVERY_NOT_ATTEMPTED;

    if (bytes_read >= 0) {
        buf[bytes_read < (int64_t)bytes_to_read ? bytes_read : (int64_t)bytes_to_read] = '\0';
        snprintf(res.description, sizeof(res.description), "Successfully read %lld bytes.", (long long)bytes_read);
    }
    free(buf);

    error_analyzer_enrich(&res);
    return res;
}

ExecutionResult syscall_execute_file_write(const char* filepath, const char* content) {
    ExecutionResult res;
    memset(&res, 0, sizeof(res));
    id_generate("f-write", res.execution_id, sizeof(res.execution_id));
    time_get_iso8601(res.timestamp, sizeof(res.timestamp));
    strncpy(res.operation, "file_write", sizeof(res.operation) - 1);
#if defined(PLATFORM_WINDOWS)
    strncpy(res.native_api, "WriteFile", sizeof(res.native_api) - 1);
#else
    strncpy(res.native_api, "write", sizeof(res.native_api) - 1);
#endif
    strncpy(res.target_path, filepath ? filepath : "", sizeof(res.target_path) - 1);

    int32_t err = 0;
    int64_t handle = platform_open_file(filepath, "w", &err);
    if (handle == -1) {
        res.return_value = -1;
        res.native_error_code = err;
        res.success = false;
        error_analyzer_enrich(&res);
        return res;
    }

    size_t len = content ? strlen(content) : 0;
    int64_t t0 = platform_get_time_us();
    int64_t bytes_written = platform_write_file(handle, content ? content : "", len, &err);
    int64_t t1 = platform_get_time_us();
    platform_close_file(handle, NULL);

    res.elapsed_us = t1 - t0;
    res.return_value = bytes_written;
    res.native_error_code = err;
    res.success = (bytes_written >= 0);
    res.recovery_status = RECOVERY_NOT_ATTEMPTED;

    if (bytes_written >= 0) {
        snprintf(res.description, sizeof(res.description), "Successfully wrote %lld bytes.", (long long)bytes_written);
    }

    error_analyzer_enrich(&res);
    return res;
}

ExecutionResult syscall_execute_file_close_invalid(int64_t fake_handle) {
    ExecutionResult res;
    memset(&res, 0, sizeof(res));
    id_generate("f-badh", res.execution_id, sizeof(res.execution_id));
    time_get_iso8601(res.timestamp, sizeof(res.timestamp));
    strncpy(res.operation, "invalid_descriptor_test", sizeof(res.operation) - 1);
#if defined(PLATFORM_WINDOWS)
    strncpy(res.native_api, "CloseHandle", sizeof(res.native_api) - 1);
#else
    strncpy(res.native_api, "close", sizeof(res.native_api) - 1);
#endif
    strncpy(res.target_path, "N/A (Descriptor: 0xDEADBEEF)", sizeof(res.target_path) - 1);

    int32_t err = 0;
    int64_t t0 = platform_get_time_us();
    bool ok = platform_close_file(fake_handle, &err);
    int64_t t1 = platform_get_time_us();

    res.elapsed_us = t1 - t0;
    res.return_value = ok ? 0 : -1;
    res.native_error_code = err;
    res.success = ok;
    res.recovery_status = RECOVERY_NOT_ATTEMPTED;

    error_analyzer_enrich(&res);
    return res;
}

ExecutionResult syscall_execute_file_missing_test(const char* custom_path) {
    char test_path[MAX_PATH_LENGTH];
    if (custom_path && strlen(custom_path) > 0) {
        strncpy(test_path, custom_path, sizeof(test_path) - 1);
    } else {
        snprintf(test_path, sizeof(test_path), "%s/non_existent_file_%lx.dat", 
                 g_app_config.temp_dir, (unsigned long)time(NULL));
    }

    // Keep controlled failure tests inside the application sandbox.
    if (platform_file_exists(test_path)) {
        snprintf(test_path, sizeof(test_path), "%s/non_existent_file_%lx.dat",
                 g_app_config.temp_dir, (unsigned long)platform_get_time_us());
    }

    return syscall_execute_file_open(test_path, "r");
}

ExecutionResult syscall_execute_file_readonly_write_test(const char* filepath) {
    char target[MAX_PATH_LENGTH];
    if (filepath && strlen(filepath) > 0) {
        strncpy(target, filepath, sizeof(target) - 1);
    } else {
        snprintf(target, sizeof(target), "%s/readonly_test_%lx.txt", g_app_config.temp_dir, (unsigned long)time(NULL));
        // Create file then open with read only or attempt writing
        int32_t err = 0;
        int64_t h = platform_open_file(target, "w", &err);
        if (h != -1) {
            platform_write_file(h, "Initial Data", 12, &err);
            platform_close_file(h, NULL);
        }
#if defined(PLATFORM_WINDOWS)
        SetFileAttributesA(target, FILE_ATTRIBUTE_READONLY);
#endif
    }

    // Intentionally attempt write opening on read-only file
    return syscall_execute_file_open(target, "w");
}

ExecutionResult syscall_execute_process(const char* exec_path, const char* args, uint32_t timeout_ms) {
    ExecutionResult res;
    memset(&res, 0, sizeof(res));
    id_generate("p-exec", res.execution_id, sizeof(res.execution_id));
    time_get_iso8601(res.timestamp, sizeof(res.timestamp));
    strncpy(res.operation, "process_create", sizeof(res.operation) - 1);
#if defined(PLATFORM_WINDOWS)
    strncpy(res.native_api, "CreateProcessA", sizeof(res.native_api) - 1);
#else
    strncpy(res.native_api, "fork+execv", sizeof(res.native_api) - 1);
#endif
    strncpy(res.target_path, exec_path ? exec_path : "", sizeof(res.target_path) - 1);

    ProcessExecutionResult proc_res;
    int32_t err = 0;
    int64_t t0 = platform_get_time_us();
    bool ok = platform_create_process(exec_path, args, timeout_ms, &proc_res, &err);
    int64_t t1 = platform_get_time_us();

    res.elapsed_us = t1 - t0;
    res.return_value = ok ? proc_res.exit_code : -1;
    res.native_error_code = err;
    res.success = ok && (proc_res.exit_code == 0);
    res.recovery_status = RECOVERY_NOT_ATTEMPTED;

    if (ok) {
        snprintf(res.description, sizeof(res.description), 
                 "Process (PID: %u) exited with code %d. Stdout: %s", 
                 proc_res.process_id, proc_res.exit_code, 
                 strlen(proc_res.stdout_captured) > 0 ? proc_res.stdout_captured : "[None]");
    }

    error_analyzer_enrich(&res);
    return res;
}

ExecutionResult syscall_execute_missing_process_test(const char* custom_path) {
    char bad_path[MAX_PATH_LENGTH];
    if (custom_path && strlen(custom_path) > 0) {
        strncpy(bad_path, custom_path, sizeof(bad_path) - 1);
    } else {
        snprintf(bad_path, sizeof(bad_path), "%s/missing_binary_%lx.exe", g_app_config.temp_dir, (unsigned long)time(NULL));
    }
    return syscall_execute_process(bad_path, NULL, 3000);
}
