#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../include/platform.h"

int64_t platform_open_file(const char* path, const char* mode, int32_t* native_error) {
    if (!path || !mode) {
        if (native_error) *native_error = ERROR_INVALID_PARAMETER;
        return -1;
    }

    DWORD desired_access = 0;
    DWORD creation_disposition = 0;
    DWORD share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;

    if (strcmp(mode, "r") == 0 || strcmp(mode, "read") == 0) {
        desired_access = GENERIC_READ;
        creation_disposition = OPEN_EXISTING;
    } else if (strcmp(mode, "w") == 0 || strcmp(mode, "write") == 0) {
        desired_access = GENERIC_WRITE;
        creation_disposition = CREATE_ALWAYS;
    } else if (strcmp(mode, "a") == 0 || strcmp(mode, "append") == 0) {
        desired_access = FILE_APPEND_DATA;
        creation_disposition = OPEN_ALWAYS;
    } else if (strcmp(mode, "rw") == 0) {
        desired_access = GENERIC_READ | GENERIC_WRITE;
        creation_disposition = OPEN_EXISTING;
    } else {
        if (native_error) *native_error = ERROR_INVALID_PARAMETER;
        return -1;
    }

    HANDLE h = CreateFileA(path, desired_access, share_mode, NULL, creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        if (native_error) *native_error = (int32_t)GetLastError();
        return -1;
    }

    if (native_error) *native_error = 0;
    return (int64_t)(intptr_t)h;
}

int64_t platform_read_file(int64_t handle, void* buffer, size_t bytes_to_read, int32_t* native_error) {
    HANDLE h = (HANDLE)(intptr_t)handle;
    if (h == NULL || h == INVALID_HANDLE_VALUE) {
        if (native_error) *native_error = ERROR_INVALID_HANDLE;
        return -1;
    }

    DWORD bytes_read = 0;
    BOOL ok = ReadFile(h, buffer, (DWORD)bytes_to_read, &bytes_read, NULL);
    if (!ok) {
        if (native_error) *native_error = (int32_t)GetLastError();
        return -1;
    }

    if (native_error) *native_error = 0;
    return (int64_t)bytes_read;
}

int64_t platform_write_file(int64_t handle, const void* buffer, size_t bytes_to_write, int32_t* native_error) {
    HANDLE h = (HANDLE)(intptr_t)handle;
    if (h == NULL || h == INVALID_HANDLE_VALUE) {
        if (native_error) *native_error = ERROR_INVALID_HANDLE;
        return -1;
    }

    DWORD bytes_written = 0;
    BOOL ok = WriteFile(h, buffer, (DWORD)bytes_to_write, &bytes_written, NULL);
    if (!ok) {
        if (native_error) *native_error = (int32_t)GetLastError();
        return -1;
    }

    if (native_error) *native_error = 0;
    return (int64_t)bytes_written;
}

bool platform_close_file(int64_t handle, int32_t* native_error) {
    HANDLE h = (HANDLE)(intptr_t)handle;
    if (h == NULL || h == INVALID_HANDLE_VALUE) {
        if (native_error) *native_error = ERROR_INVALID_HANDLE;
        return false;
    }

    BOOL ok = CloseHandle(h);
    if (!ok) {
        if (native_error) *native_error = (int32_t)GetLastError();
        return false;
    }

    if (native_error) *native_error = 0;
    return true;
}

bool platform_file_exists(const char* path) {
    if (!path) return false;
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool platform_delete_file(const char* path) {
    if (!path) return false;
    return DeleteFileA(path) != 0;
}

int64_t platform_get_file_size(const char* path) {
    if (!path) return -1;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &fad)) {
        return -1;
    }
    LARGE_INTEGER size;
    size.LowPart = fad.nFileSizeLow;
    size.HighPart = fad.nFileSizeHigh;
    return (int64_t)size.QuadPart;
}
