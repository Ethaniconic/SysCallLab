#ifndef SYSCALL_PLATFORM_H
#define SYSCALL_PLATFORM_H

#include "common.h"

typedef struct {
    char os_name[64];
    char os_version[128];
    char architecture[64];
    char compiler_info[128];
    char hostname[128];
    uint32_t process_id;
    uint32_t thread_id;
    uint64_t total_ram_mb;
    uint64_t free_ram_mb;
    int cpu_cores;
} SystemInfo;

// Platform lifecycle
bool platform_init(void);
void platform_cleanup(void);

// System info
void platform_get_system_info(SystemInfo* info);

// High-resolution timing
int64_t platform_get_time_us(void);

// Sleep helper
void platform_sleep_ms(uint32_t ms);

// File operations
int64_t platform_open_file(const char* path, const char* mode, int32_t* native_error);
int64_t platform_read_file(int64_t handle, void* buffer, size_t bytes_to_read, int32_t* native_error);
int64_t platform_write_file(int64_t handle, const void* buffer, size_t bytes_to_write, int32_t* native_error);
bool platform_close_file(int64_t handle, int32_t* native_error);
bool platform_file_exists(const char* path);
bool platform_delete_file(const char* path);
int64_t platform_get_file_size(const char* path);

// Process operations
typedef struct {
    uint32_t process_id;
    int32_t exit_code;
    bool exited;
    char stdout_captured[MAX_BUFFER_SIZE];
    char stderr_captured[MAX_BUFFER_SIZE];
} ProcessExecutionResult;

bool platform_create_process(const char* executable_path, const char* arguments, 
                            uint32_t timeout_ms, ProcessExecutionResult* result, int32_t* native_error);

// Native error inspection and translation
void platform_format_error(int32_t native_error, char* out_desc, size_t max_len);
const char* platform_get_error_identifier(int32_t native_error);
ErrorCategory platform_categorize_error(int32_t native_error);

#endif // SYSCALL_PLATFORM_H
