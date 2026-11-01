#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

int main(int argc, char* argv[]) {
    printf("[HELPER_CHILD] Child process started with %d arguments.\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("[HELPER_CHILD] Arg[%d]: %s\n", i, argv[i]);
    }

    int exit_code = 0;
    if (argc > 1) {
        exit_code = atoi(argv[1]);
    }

    if (argc > 2 && atoi(argv[2]) > 0) {
        int sleep_ms = atoi(argv[2]);
        printf("[HELPER_CHILD] Sleeping for %d ms...\n", sleep_ms);
#if defined(_WIN32)
        Sleep(sleep_ms);
#else
        usleep(sleep_ms * 1000);
#endif
    }

    printf("[HELPER_CHILD] Terminating with exit code %d\n", exit_code);
    return exit_code;
}
