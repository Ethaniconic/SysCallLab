#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winerror.h>
#include <stdio.h>
#include "../../include/platform.h"

#ifndef PROCESSOR_ARCHITECTURE_ARM64
#define PROCESSOR_ARCHITECTURE_ARM64 12
#endif

void platform_format_error(int32_t native_error, char* out_desc, size_t max_len) {
    if (!out_desc || max_len == 0) return;
    if (native_error == 0) {
        snprintf(out_desc, max_len, "The operation completed successfully.");
        return;
    }

    LPSTR message_buffer = NULL;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD)native_error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&message_buffer,
        0,
        NULL
    );

    if (size > 0 && message_buffer) {
        // Strip trailing \r\n
        while (size > 0 && (message_buffer[size - 1] == '\r' || message_buffer[size - 1] == '\n')) {
            message_buffer[--size] = '\0';
        }
        snprintf(out_desc, max_len, "%s", message_buffer);
        LocalFree(message_buffer);
    } else {
        snprintf(out_desc, max_len, "Win32 System Error Code: %d (0x%08X)", native_error, native_error);
    }
}

const char* platform_get_error_identifier(int32_t native_error) {
    switch (native_error) {
        case ERROR_SUCCESS: return "ERROR_SUCCESS";
        case ERROR_FILE_NOT_FOUND: return "ERROR_FILE_NOT_FOUND";
        case ERROR_PATH_NOT_FOUND: return "ERROR_PATH_NOT_FOUND";
        case ERROR_ACCESS_DENIED: return "ERROR_ACCESS_DENIED";
        case ERROR_INVALID_HANDLE: return "ERROR_INVALID_HANDLE";
        case ERROR_NOT_ENOUGH_MEMORY: return "ERROR_NOT_ENOUGH_MEMORY";
        case ERROR_SHARING_VIOLATION: return "ERROR_SHARING_VIOLATION";
        case ERROR_LOCK_VIOLATION: return "ERROR_LOCK_VIOLATION";
        case ERROR_HANDLE_EOF: return "ERROR_HANDLE_EOF";
        case ERROR_INVALID_PARAMETER: return "ERROR_INVALID_PARAMETER";
        case ERROR_FILE_EXISTS: return "ERROR_FILE_EXISTS";
        case ERROR_BAD_EXE_FORMAT: return "ERROR_BAD_EXE_FORMAT";
        case ERROR_OPERATION_ABORTED: return "ERROR_OPERATION_ABORTED";
        default: return "WIN32_ERROR";
    }
}

ErrorCategory platform_categorize_error(int32_t native_error) {
    switch (native_error) {
        case 0:
            return ERR_CAT_NONE;
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return ERR_CAT_FILE_NOT_FOUND;
        case ERROR_ACCESS_DENIED:
        case ERROR_SHARING_VIOLATION:
        case ERROR_LOCK_VIOLATION:
            return ERR_CAT_PERMISSION_DENIED;
        case ERROR_INVALID_HANDLE:
            return ERR_CAT_INVALID_DESCRIPTOR;
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:
            return ERR_CAT_RESOURCE_EXHAUSTION;
        case ERROR_OPERATION_ABORTED:
            return ERR_CAT_INTERRUPTED_OPERATION;
        case ERROR_BAD_EXE_FORMAT:
            return ERR_CAT_PROCESS_CREATION_FAILURE;
        case ERROR_INVALID_PARAMETER:
        case ERROR_BAD_ARGUMENTS:
            return ERR_CAT_INVALID_ARGUMENT;
        default:
            return ERR_CAT_UNKNOWN_ERROR;
    }
}

bool platform_init(void) {
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    return (res == 0);
}

void platform_cleanup(void) {
    WSACleanup();
}

int64_t platform_get_time_us(void) {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (int64_t)((counter.QuadPart * 1000000) / freq.QuadPart);
}

void platform_sleep_ms(uint32_t ms) {
    Sleep(ms);
}

void platform_get_system_info(SystemInfo* info) {
    if (!info) return;
    memset(info, 0, sizeof(SystemInfo));

    snprintf(info->os_name, sizeof(info->os_name), "Windows");

    // Real system architecture
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            snprintf(info->architecture, sizeof(info->architecture), "x86_64 (AMD64)");
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            snprintf(info->architecture, sizeof(info->architecture), "ARM64");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            snprintf(info->architecture, sizeof(info->architecture), "x86 (32-bit)");
            break;
        default:
            snprintf(info->architecture, sizeof(info->architecture), "Unknown Architecture");
            break;
    }
    info->cpu_cores = (int)si.dwNumberOfProcessors;

    // Real memory status
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memStatus)) {
        info->total_ram_mb = (uint64_t)(memStatus.ullTotalPhys / (1024 * 1024));
        info->free_ram_mb = (uint64_t)(memStatus.ullAvailPhys / (1024 * 1024));
    }

    // Hostname
    DWORD host_len = sizeof(info->hostname);
    GetComputerNameA(info->hostname, &host_len);

    // Process & Thread IDs
    info->process_id = (uint32_t)GetCurrentProcessId();
    info->thread_id = (uint32_t)GetCurrentThreadId();

    // OS version
    snprintf(info->os_version, sizeof(info->os_version), "Windows NT (Kernel %lu.%lu)", 
             (unsigned long)LOBYTE(LOWORD(GetVersion())), (unsigned long)HIBYTE(LOWORD(GetVersion())));

    // Compiler Info
#if defined(__GNUC__)
    snprintf(info->compiler_info, sizeof(info->compiler_info), "GCC %d.%d.%d (MinGW)", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    snprintf(info->compiler_info, sizeof(info->compiler_info), "MSVC %d", _MSC_VER);
#elif defined(__clang__)
    snprintf(info->compiler_info, sizeof(info->compiler_info), "Clang %s", __clang_version__);
#else
    snprintf(info->compiler_info, sizeof(info->compiler_info), "Unknown C Compiler");
#endif
}
