#if defined(__APPLE__)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <time.h>
#include "../../include/platform.h"

void platform_format_error(int32_t native_error, char* out_desc, size_t max_len) {
    if (!out_desc || max_len == 0) return;
    if (native_error == 0) {
        snprintf(out_desc, max_len, "Success.");
        return;
    }
    const char* str = strerror(native_error);
    if (str) {
        snprintf(out_desc, max_len, "%s", str);
    } else {
        snprintf(out_desc, max_len, "macOS Darwin POSIX code %d", native_error);
    }
}

const char* platform_get_error_identifier(int32_t native_error) {
    switch (native_error) {
        case 0: return "SUCCESS";
        case ENOENT: return "ENOENT";
        case EACCES: return "EACCES";
        case EPERM: return "EPERM";
        case EBADF: return "EBADF";
        case ENOMEM: return "ENOMEM";
        case EINTR: return "EINTR";
        case EINVAL: return "EINVAL";
        case ENOEXEC: return "ENOEXEC";
        default: return "DARWIN_ERROR";
    }
}

ErrorCategory platform_categorize_error(int32_t native_error) {
    switch (native_error) {
        case 0: return ERR_CAT_NONE;
        case ENOENT: return ERR_CAT_FILE_NOT_FOUND;
        case EACCES:
        case EPERM: return ERR_CAT_PERMISSION_DENIED;
        case EBADF: return ERR_CAT_INVALID_DESCRIPTOR;
        case ENOMEM: return ERR_CAT_RESOURCE_EXHAUSTION;
        case EINTR: return ERR_CAT_INTERRUPTED_OPERATION;
        case ENOEXEC: return ERR_CAT_PROCESS_CREATION_FAILURE;
        case EINVAL: return ERR_CAT_INVALID_ARGUMENT;
        default: return ERR_CAT_UNKNOWN_ERROR;
    }
}

bool platform_init(void) {
    return true;
}

void platform_cleanup(void) {
}

int64_t platform_get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((int64_t)ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}

void platform_sleep_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void platform_get_system_info(SystemInfo* info) {
    if (!info) return;
    memset(info, 0, sizeof(SystemInfo));
    snprintf(info->os_name, sizeof(info->os_name), "macOS");

    struct utsname un;
    if (uname(&un) == 0) {
        snprintf(info->os_version, sizeof(info->os_version), "Darwin %s", un.release);
        snprintf(info->architecture, sizeof(info->architecture), "%s", un.machine);
        snprintf(info->hostname, sizeof(info->hostname), "%s", un.nodename);
    }

    int mib[2] = {CTL_HW, HW_MEMSIZE};
    uint64_t mem = 0;
    size_t len = sizeof(mem);
    if (sysctl(mib, 2, &mem, &len, NULL, 0) == 0) {
        info->total_ram_mb = mem / (1024 * 1024);
        info->free_ram_mb = info->total_ram_mb / 2; // Approximation on Darwin
    }

    info->cpu_cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    info->process_id = (uint32_t)getpid();
    info->thread_id = (uint32_t)getpid();

#if defined(__clang__)
    snprintf(info->compiler_info, sizeof(info->compiler_info), "Apple Clang %s", __clang_version__);
#elif defined(__GNUC__)
    snprintf(info->compiler_info, sizeof(info->compiler_info), "GCC %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
    snprintf(info->compiler_info, sizeof(info->compiler_info), "C Compiler");
#endif
}
#endif
