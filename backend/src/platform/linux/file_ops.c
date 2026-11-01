#if defined(__linux__)
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include "../../include/platform.h"

int64_t platform_open_file(const char* path, const char* mode, int32_t* native_error) {
    if (!path || !mode) {
        if (native_error) *native_error = EINVAL;
        return -1;
    }
    int flags = 0;
    if (strcmp(mode, "r") == 0 || strcmp(mode, "read") == 0) {
        flags = O_RDONLY;
    } else if (strcmp(mode, "w") == 0 || strcmp(mode, "write") == 0) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "a") == 0 || strcmp(mode, "append") == 0) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    } else if (strcmp(mode, "rw") == 0) {
        flags = O_RDWR;
    } else {
        if (native_error) *native_error = EINVAL;
        return -1;
    }

    int fd = open(path, flags, 0644);
    if (fd < 0) {
        if (native_error) *native_error = errno;
        return -1;
    }
    if (native_error) *native_error = 0;
    return (int64_t)fd;
}

int64_t platform_read_file(int64_t handle, void* buffer, size_t bytes_to_read, int32_t* native_error) {
    if (handle < 0 || !buffer) {
        if (native_error) *native_error = EBADF;
        return -1;
    }
    ssize_t n = read((int)handle, buffer, bytes_to_read);
    if (n < 0) {
        if (native_error) *native_error = errno;
        return -1;
    }
    if (native_error) *native_error = 0;
    return (int64_t)n;
}

int64_t platform_write_file(int64_t handle, const void* buffer, size_t bytes_to_write, int32_t* native_error) {
    if (handle < 0 || !buffer) {
        if (native_error) *native_error = EBADF;
        return -1;
    }
    ssize_t n = write((int)handle, buffer, bytes_to_write);
    if (n < 0) {
        if (native_error) *native_error = errno;
        return -1;
    }
    if (native_error) *native_error = 0;
    return (int64_t)n;
}

bool platform_close_file(int64_t handle, int32_t* native_error) {
    if (handle < 0) {
        if (native_error) *native_error = EBADF;
        return false;
    }
    int res = close((int)handle);
    if (res < 0) {
        if (native_error) *native_error = errno;
        return false;
    }
    if (native_error) *native_error = 0;
    return true;
}

bool platform_file_exists(const char* path) {
    if (!path) return false;
    struct stat st;
    return (stat(path, &st) == 0 && !S_ISDIR(st.st_mode));
}

bool platform_delete_file(const char* path) {
    if (!path) return false;
    return (unlink(path) == 0);
}

int64_t platform_get_file_size(const char* path) {
    if (!path) return -1;
    struct stat st;
    if (stat(path, &st) == 0) return (int64_t)st.st_size;
    return -1;
}
#endif
