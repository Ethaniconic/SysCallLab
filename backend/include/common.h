#ifndef SYSCALL_COMMON_H
#define SYSCALL_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define SYSCALL_APP_NAME "SysCallLab"
#define SYSCALL_APP_VERSION "1.0.0"
#define SYSCALL_APP_SUBTITLE "Cross-Platform System Call Analysis and Error Recovery Toolkit"
#define SYSCALL_DEFAULT_PORT 8080
#define SYSCALL_DEFAULT_HOST "127.0.0.1"

#define MAX_PATH_LENGTH 1024
#define MAX_BUFFER_SIZE 4096
#define MAX_ERROR_DESC 512
#define MAX_ID_LENGTH 64

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_NAME "windows"
#elif defined(__APPLE__) || defined(__MACH__)
    #define PLATFORM_MACOS 1
    #define PLATFORM_NAME "macos"
#elif defined(__linux__)
    #define PLATFORM_LINUX 1
    #define PLATFORM_NAME "linux"
#else
    #define PLATFORM_UNKNOWN 1
    #define PLATFORM_NAME "unknown"
#endif

typedef enum {
    ERR_CAT_NONE = 0,
    ERR_CAT_FILE_NOT_FOUND,
    ERR_CAT_PERMISSION_DENIED,
    ERR_CAT_INVALID_DESCRIPTOR,
    ERR_CAT_RESOURCE_EXHAUSTION,
    ERR_CAT_INTERRUPTED_OPERATION,
    ERR_CAT_PROCESS_CREATION_FAILURE,
    ERR_CAT_EXECUTION_FAILURE,
    ERR_CAT_INVALID_ARGUMENT,
    ERR_CAT_MEMORY_ALLOCATION_FAILURE,
    ERR_CAT_UNKNOWN_ERROR
} ErrorCategory;

typedef enum {
    RECOVERY_NOT_ATTEMPTED = 0,
    RECOVERY_ATTEMPTED_SUCCESS,
    RECOVERY_ATTEMPTED_FAILED,
    RECOVERY_NOT_APPLICABLE
} RecoveryStatus;

typedef struct {
    char execution_id[MAX_ID_LENGTH];
    char timestamp[64];
    char operation[64];
    char native_api[64];
    int64_t return_value;
    int32_t native_error_code;
    char error_identifier[64];
    ErrorCategory category;
    char error_category_str[64];
    char description[MAX_ERROR_DESC];
    char possible_cause[MAX_ERROR_DESC];
    char suggested_recovery[MAX_ERROR_DESC];
    char target_path[MAX_PATH_LENGTH];
    bool success;
    RecoveryStatus recovery_status;
    char recovery_notes[MAX_ERROR_DESC];
    int64_t elapsed_us;
} ExecutionResult;

const char* error_category_to_string(ErrorCategory category);
ErrorCategory string_to_error_category(const char* str);
const char* recovery_status_to_string(RecoveryStatus status);
RecoveryStatus string_to_recovery_status(const char* str);

#endif // SYSCALL_COMMON_H
