#if defined(__linux__)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
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
        snprintf(out_desc, max_len, "POSIX error code %d", native_error);
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
        default: return "POSIX_ERROR";
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
    snprintf(info->os_name, sizeof(info->os_name), "Linux");

    struct utsname un;
    if (uname(&un) == 0) {
        snprintf(info->os_version, sizeof(info->os_version), "%s %s", un.sysname, un.release);
        snprintf(info->architecture, sizeof(info->architecture), "%s", un.machine);
        snprintf(info->hostname, sizeof(info->hostname), "%s", un.nodename);
    }

    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        info->total_ram_mb = (uint64_t)(si.totalram * si.mem_unit) / (1024 * 1024);
        info->free_ram_mb = (uint64_t)(si.freeram * si.mem_unit) / (1024 * 1024);
    }

    info->cpu_cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    info->process_id = (uint32_t)getpid();
    info->thread_id = (uint32_t)getpid();

#if defined(__GNUC__)
    snprintf(info->compiler_info, sizeof(info->compiler_info), "GCC %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
    snprintf(info->compiler_info, sizeof(info->compiler_info), "C Compiler");
#endif
}
#endif
