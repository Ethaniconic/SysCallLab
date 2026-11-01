#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/recovery.h"
#include "../../include/database.h"
#include "../../include/platform.h"
#include "../../include/logger.h"
#include "../../include/config.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#endif

bool recovery_execute(const char* execution_id, const char* strategy, bool confirmed, RecoveryExecutionResult* out_result) {
    if (!out_result) return false;
    memset(out_result, 0, sizeof(RecoveryExecutionResult));
    strncpy(out_result->execution_id, execution_id ? execution_id : "", sizeof(out_result->execution_id) - 1);
    strncpy(out_result->strategy_name, strategy ? strategy : "auto_heal", sizeof(out_result->strategy_name) - 1);
    out_result->user_confirmed = confirmed;

    if (!confirmed) {
        out_result->success = false;
        strncpy(out_result->details, "Recovery aborted: User explicit confirmation is required before attempting OS recovery.", sizeof(out_result->details) - 1);
        return false;
    }

    ExecutionResult orig;
    if (!database_get_execution_by_id(execution_id, &orig)) {
        out_result->success = false;
        strncpy(out_result->details, "Recovery failed: Original execution ID not found in database records.", sizeof(out_result->details) - 1);
        return false;
    }

    bool recovered = false;

    // Strategy 1: Missing file recovery -> Create empty file in designated path and verify
    if (orig.category == ERR_CAT_FILE_NOT_FOUND) {
        if (strlen(orig.target_path) > 0) {
            int32_t err = 0;
            int64_t h = platform_open_file(orig.target_path, "w", &err);
            if (h != -1) {
                const char* init_content = "[SysCallLab Recovery Initialized File Content]\n";
                platform_write_file(h, init_content, strlen(init_content), NULL);
                platform_close_file(h, NULL);

                // Verify recovery
                int64_t verify_h = platform_open_file(orig.target_path, "r", &err);
                if (verify_h != -1) {
                    platform_close_file(verify_h, NULL);
                    recovered = true;
                    snprintf(out_result->details, sizeof(out_result->details), 
                             "Recovery verified: Successfully created test file '%s' and confirmed readability via native kernel handle.", orig.target_path);
                } else {
                    snprintf(out_result->details, sizeof(out_result->details), 
                             "Recovery verification failed: Created file could not be reopened.");
                }
            } else {
                snprintf(out_result->details, sizeof(out_result->details), 
                         "Recovery failed to create file '%s' (Error Code: %d).", orig.target_path, err);
            }
        }
    }
    // Strategy 2: Permission denied on read-only file -> Clear read-only attribute and verify write
    else if (orig.category == ERR_CAT_PERMISSION_DENIED) {
        if (strlen(orig.target_path) > 0 && platform_file_exists(orig.target_path)) {
#if defined(PLATFORM_WINDOWS)
            SetFileAttributesA(orig.target_path, FILE_ATTRIBUTE_NORMAL);
#endif
            int32_t err = 0;
            int64_t h = platform_open_file(orig.target_path, "w", &err);
            if (h != -1) {
                platform_write_file(h, "Recovery Write Test Succeeded", 29, NULL);
                platform_close_file(h, NULL);
                recovered = true;
                snprintf(out_result->details, sizeof(out_result->details), 
                         "Recovery verified: Reset read-only attribute on '%s' and confirmed write capability.", orig.target_path);
            } else {
                snprintf(out_result->details, sizeof(out_result->details), 
                         "Recovery failed: Unable to gain write access after attribute reset (Error Code: %d).", err);
            }
        }
    }
    // Strategy 3: Invalid descriptor -> Demonstrate acquiring and operating on valid handle
    else if (orig.category == ERR_CAT_INVALID_DESCRIPTOR) {
        char valid_path[MAX_PATH_LENGTH];
        snprintf(valid_path, sizeof(valid_path), "%s/valid_descriptor_%lx.tmp", g_app_config.temp_dir, (unsigned long)time(NULL));
        int32_t err = 0;
        int64_t valid_h = platform_open_file(valid_path, "w", &err);
        if (valid_h != -1) {
            bool close_ok = platform_close_file(valid_h, &err);
            platform_delete_file(valid_path);
            if (close_ok) {
                recovered = true;
                snprintf(out_result->details, sizeof(out_result->details), 
                         "Recovery verified: Acquired genuine kernel descriptor and safely executed CloseHandle/close without errors.");
            }
        }
    }
    // Strategy 4: Missing executable -> Point to verified helper executable
    else if (orig.category == ERR_CAT_PROCESS_CREATION_FAILURE) {
        char helper_path[MAX_PATH_LENGTH];
#if defined(PLATFORM_WINDOWS)
        snprintf(helper_path, sizeof(helper_path), "build\\bin\\test_child.exe");
#else
        snprintf(helper_path, sizeof(helper_path), "build/bin/test_child");
#endif
        if (platform_file_exists(helper_path)) {
            ProcessExecutionResult proc;
            int32_t err = 0;
            if (platform_create_process(helper_path, "0 10", 3000, &proc, &err)) {
                recovered = (proc.exit_code == 0);
                snprintf(out_result->details, sizeof(out_result->details), 
                         "Recovery verified: Redirected execution to authentic helper binary '%s' (PID: %u, Exit: %d).", 
                         helper_path, proc.process_id, proc.exit_code);
            }
        } else {
            snprintf(out_result->details, sizeof(out_result->details), 
                     "Recovery failed: Alternative helper binary '%s' not yet compiled.", helper_path);
        }
    } else {
        snprintf(out_result->details, sizeof(out_result->details), 
                 "No automated recovery handler implemented for category: %s", orig.error_category_str);
    }

    out_result->success = recovered;
    database_update_recovery(execution_id, recovered ? RECOVERY_ATTEMPTED_SUCCESS : RECOVERY_ATTEMPTED_FAILED, out_result->details);
    logger_log(LOG_LEVEL_INFO, "Recovery attempt on [%s]: %s - %s", execution_id, recovered ? "SUCCESS" : "FAILED", out_result->details);

    return recovered;
}
