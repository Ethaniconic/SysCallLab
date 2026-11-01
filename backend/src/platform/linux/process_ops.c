#if defined(__linux__)
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "../../include/platform.h"

bool platform_create_process(const char* executable_path, const char* arguments, 
                            uint32_t timeout_ms, ProcessExecutionResult* result, int32_t* native_error) {
    if (!result) return false;
    memset(result, 0, sizeof(ProcessExecutionResult));

    int pipe_out[2];
    int pipe_err[2];

    if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0) {
        if (native_error) *native_error = errno;
        return false;
    }

    pid_t pid = fork();
    if (pid < 0) {
        if (native_error) *native_error = errno;
        close(pipe_out[0]); close(pipe_out[1]);
        close(pipe_err[0]); close(pipe_err[1]);
        return false;
    }

    if (pid == 0) {
        // Child
        close(pipe_out[0]);
        close(pipe_err[0]);
        dup2(pipe_out[1], STDOUT_FILENO);
        dup2(pipe_err[1], STDERR_FILENO);
        close(pipe_out[1]);
        close(pipe_err[1]);

        char* argv[32];
        int idx = 0;
        argv[idx++] = (char*)executable_path;
        // Parse basic arguments
        char args_copy[MAX_BUFFER_SIZE];
        if (arguments && strlen(arguments) > 0) {
            strncpy(args_copy, arguments, sizeof(args_copy) - 1);
            char* tok = strtok(args_copy, " ");
            while (tok && idx < 30) {
                argv[idx++] = tok;
                tok = strtok(NULL, " ");
            }
        }
        argv[idx] = NULL;

        execv(executable_path, argv);
        _exit(127);
    }

    // Parent
    close(pipe_out[1]);
    close(pipe_err[1]);
    result->process_id = (uint32_t)pid;

    ssize_t out_n = read(pipe_out[0], result->stdout_captured, sizeof(result->stdout_captured) - 1);
    if (out_n > 0) result->stdout_captured[out_n] = '\0';
    close(pipe_out[0]);

    ssize_t err_n = read(pipe_err[0], result->stderr_captured, sizeof(result->stderr_captured) - 1);
    if (err_n > 0) result->stderr_captured[err_n] = '\0';
    close(pipe_err[0]);

    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        result->exit_code = WEXITSTATUS(status);
        result->exited = true;
    } else {
        result->exit_code = -1;
        result->exited = false;
    }

    if (native_error) *native_error = 0;
    return true;
}
#endif
