#include <stdio.h>
#include <string.h>
#include "../../include/test_runner.h"
#include "../../include/syscall_runner.h"
#include "../../include/database.h"
#include "../../include/config.h"

bool test_runner_run_suite(const char* category, TestSuiteSummary* summary) {
    if (!summary) return false;
    memset(summary, 0, sizeof(TestSuiteSummary));

    bool run_all = (!category || strlen(category) == 0 || strcmp(category, "all") == 0);
    bool run_file = (run_all || strcmp(category, "file") == 0);
    bool run_process = (run_all || strcmp(category, "process") == 0);
    bool run_error = (run_all || strcmp(category, "error") == 0);

    int idx = 0;

    // Test 1: File Open & Write Verification
    if (run_file && idx < 32) {
        TestCaseResult* t = &summary->results[idx++];
        strncpy(t->test_id, "TC-FILE-01", sizeof(t->test_id) - 1);
        strncpy(t->name, "Genuine File Creation and Write", sizeof(t->name) - 1);
        strncpy(t->category, "File Operations", sizeof(t->category) - 1);
        strncpy(t->operation, "file_write", sizeof(t->operation) - 1);
        strncpy(t->expected_outcome, "Write operation succeeds with positive bytes written", sizeof(t->expected_outcome) - 1);

        char test_file[MAX_PATH_LENGTH];
        snprintf(test_file, sizeof(test_file), "%s/test_write.tmp", g_app_config.temp_dir);
        ExecutionResult res = syscall_execute_file_write(test_file, "SysCallLab Real File Test 123");
        database_insert_execution(&res);

        t->actual_error_code = res.native_error_code;
        t->passed = res.success;
        snprintf(t->actual_outcome, sizeof(t->actual_outcome), "%s", res.description);
        snprintf(t->details, sizeof(t->details), "Return value: %lld, Error: %d", (long long)res.return_value, res.native_error_code);
    }

    // Test 2: Controlled Missing File Detection (Expected failure must PASS test)
    if ((run_file || run_error) && idx < 32) {
        TestCaseResult* t = &summary->results[idx++];
        strncpy(t->test_id, "TC-FILE-02", sizeof(t->test_id) - 1);
        strncpy(t->name, "Missing File Detection Test", sizeof(t->name) - 1);
        strncpy(t->category, "File Operations", sizeof(t->category) - 1);
        strncpy(t->operation, "file_open_missing", sizeof(t->operation) - 1);
        strncpy(t->expected_outcome, "File open fails and returns FILE_NOT_FOUND error code", sizeof(t->expected_outcome) - 1);

        ExecutionResult res = syscall_execute_file_missing_test(NULL);
        database_insert_execution(&res);

        t->actual_error_code = res.native_error_code;
        // Test PASSES when failure condition is properly triggered and detected
        t->passed = (!res.success && res.category == ERR_CAT_FILE_NOT_FOUND);
        snprintf(t->actual_outcome, sizeof(t->actual_outcome), "System call failed as expected: %s", res.description);
        snprintf(t->details, sizeof(t->details), "Detected error category: %s (Code: %d)", res.error_category_str, res.native_error_code);
    }

    // Test 3: Invalid Descriptor Test (Expected failure must PASS test)
    if ((run_file || run_error) && idx < 32) {
        TestCaseResult* t = &summary->results[idx++];
        strncpy(t->test_id, "TC-FILE-03", sizeof(t->test_id) - 1);
        strncpy(t->name, "Invalid Descriptor Rejection", sizeof(t->name) - 1);
        strncpy(t->category, "File Operations", sizeof(t->category) - 1);
        strncpy(t->operation, "close_invalid_handle", sizeof(t->operation) - 1);
        strncpy(t->expected_outcome, "Kernel rejects invalid descriptor with INVALID_DESCRIPTOR error", sizeof(t->expected_outcome) - 1);

        ExecutionResult res = syscall_execute_file_close_invalid((int64_t)0xDEADBEEF);
        database_insert_execution(&res);

        t->actual_error_code = res.native_error_code;
        t->passed = (!res.success && res.category == ERR_CAT_INVALID_DESCRIPTOR);
        snprintf(t->actual_outcome, sizeof(t->actual_outcome), "Close rejected with code: %d", res.native_error_code);
        snprintf(t->details, sizeof(t->details), "Category: %s (%s)", res.error_category_str, res.description);
    }

    // Test 4: Missing Executable Execution (Expected failure must PASS test)
    if ((run_process || run_error) && idx < 32) {
        TestCaseResult* t = &summary->results[idx++];
        strncpy(t->test_id, "TC-PROC-01", sizeof(t->test_id) - 1);
        strncpy(t->name, "Missing Executable Handling", sizeof(t->name) - 1);
        strncpy(t->category, "Process Operations", sizeof(t->category) - 1);
        strncpy(t->operation, "process_create_missing", sizeof(t->operation) - 1);
        strncpy(t->expected_outcome, "Process creation fails with PROCESS_CREATION_FAILURE or FILE_NOT_FOUND", sizeof(t->expected_outcome) - 1);

        ExecutionResult res = syscall_execute_missing_process_test(NULL);
        database_insert_execution(&res);

        t->actual_error_code = res.native_error_code;
        t->passed = (!res.success && (res.category == ERR_CAT_PROCESS_CREATION_FAILURE || res.category == ERR_CAT_FILE_NOT_FOUND));
        snprintf(t->actual_outcome, sizeof(t->actual_outcome), "Creation rejected with code: %d", res.native_error_code);
        snprintf(t->details, sizeof(t->details), "%s", res.description);
    }

    summary->total_tests = idx;
    for (int i = 0; i < idx; i++) {
        if (summary->results[i].passed) {
            summary->passed_tests++;
        } else {
            summary->failed_tests++;
        }
    }

    return true;
}
