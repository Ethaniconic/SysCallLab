#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../include/platform.h"

bool platform_create_process(const char* executable_path, const char* arguments, 
                            uint32_t timeout_ms, ProcessExecutionResult* result, int32_t* native_error) {
    if (!result) return false;
    memset(result, 0, sizeof(ProcessExecutionResult));

    if (!executable_path || strlen(executable_path) == 0) {
        if (native_error) *native_error = ERROR_BAD_ARGUMENTS;
        return false;
    }

    // Build command line safely
    char cmd_line[MAX_BUFFER_SIZE];
    if (arguments && strlen(arguments) > 0) {
        snprintf(cmd_line, sizeof(cmd_line), "\"%s\" %s", executable_path, arguments);
    } else {
        snprintf(cmd_line, sizeof(cmd_line), "\"%s\"", executable_path);
    }

    HANDLE hStdOutRead = NULL, hStdOutWrite = NULL;
    HANDLE hStdErrRead = NULL, hStdErrWrite = NULL;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
        if (native_error) *native_error = (int32_t)GetLastError();
        return false;
    }
    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

    if (!CreatePipe(&hStdErrRead, &hStdErrWrite, &sa, 0)) {
        if (native_error) *native_error = (int32_t)GetLastError();
        CloseHandle(hStdOutRead);
        CloseHandle(hStdOutWrite);
        return false;
    }
    SetHandleInformation(hStdErrRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = hStdOutWrite;
    si.hStdError = hStdErrWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&pi, sizeof(pi));

    BOOL success = CreateProcessA(
        NULL,
        cmd_line,
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );

    // Close write ends of pipes in parent immediately
    CloseHandle(hStdOutWrite);
    CloseHandle(hStdErrWrite);

    if (!success) {
        if (native_error) *native_error = (int32_t)GetLastError();
        CloseHandle(hStdOutRead);
        CloseHandle(hStdErrRead);
        return false;
    }

    result->process_id = (uint32_t)pi.dwProcessId;

    // Wait for process with timeout before reading from pipes
    DWORD wait_result = WaitForSingleObject(pi.hProcess, timeout_ms > 0 ? timeout_ms : 5000);
    if (wait_result == WAIT_OBJECT_0) {
        DWORD exit_code = 0;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        result->exit_code = (int32_t)exit_code;
        result->exited = true;
    } else if (wait_result == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 101);
        result->exit_code = 101;
        result->exited = false;
        strncpy(result->stderr_captured, "[Process timed out and terminated]", sizeof(result->stderr_captured) - 1);
    }

    // Now safely read outputs without blocking indefinitely
    DWORD avail = 0;
    if (PeekNamedPipe(hStdOutRead, NULL, 0, NULL, &avail, NULL) && avail > 0) {
        DWORD bytes_read = 0;
        ReadFile(hStdOutRead, result->stdout_captured, sizeof(result->stdout_captured) - 1, &bytes_read, NULL);
        result->stdout_captured[bytes_read] = '\0';
    }

    avail = 0;
    if (PeekNamedPipe(hStdErrRead, NULL, 0, NULL, &avail, NULL) && avail > 0) {
        DWORD bytes_read = 0;
        ReadFile(hStdErrRead, result->stderr_captured, sizeof(result->stderr_captured) - 1, &bytes_read, NULL);
        result->stderr_captured[bytes_read] = '\0';
    }

    CloseHandle(hStdOutRead);
    CloseHandle(hStdErrRead);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (native_error) *native_error = 0;
    return true;
}
