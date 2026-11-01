#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../include/common.h"
#include "../include/config.h"
#include "../include/platform.h"
#include "../include/syscall_runner.h"
#include "../include/error_analyzer.h"
#include "../include/database.h"
#include "../include/recovery.h"

static void test_platform_info(void) {
    SystemInfo info;
    platform_get_system_info(&info);
    printf("[TEST] OS Name: %s, Arch: %s, CPU cores: %d\n", info.os_name, info.architecture, info.cpu_cores);
    assert(strlen(info.os_name) > 0);
    assert(info.cpu_cores > 0);
    printf("[PASS] test_platform_info\n");
}

static void test_missing_file_detection(void) {
    ExecutionResult res = syscall_execute_file_missing_test(NULL);
    printf("[TEST] Missing file test: Success=%d, ErrorCode=%d, Category=%s\n", res.success, res.native_error_code, res.error_category_str);
    assert(!res.success);
    assert(res.category == ERR_CAT_FILE_NOT_FOUND);
    assert(res.native_error_code != 0);
    printf("[PASS] test_missing_file_detection (Correctly identified missing file error)\n");
}

static void test_file_write_and_read(void) {
    char path[] = "data/unit_test_file.txt";
    ExecutionResult wres = syscall_execute_file_write(path, "SysCallLab Integration Test");
    assert(wres.success);
    assert(wres.return_value > 0);

    ExecutionResult rres = syscall_execute_file_read(path, 64);
    assert(rres.success);
    assert(rres.return_value > 0);
    platform_delete_file(path);
    printf("[PASS] test_file_write_and_read\n");
}

static void test_invalid_descriptor(void) {
    ExecutionResult res = syscall_execute_file_close_invalid((int64_t)0xDEADBEEF);
    assert(!res.success);
    assert(res.category == ERR_CAT_INVALID_DESCRIPTOR);
    printf("[PASS] test_invalid_descriptor\n");
}

static void test_database_persistence(void) {
    database_init("data/test_db.db");
    ExecutionResult res = syscall_execute_file_missing_test(NULL);
    bool inserted = database_insert_execution(&res);
    assert(inserted);

    ExecutionResult fetched;
    bool found = database_get_execution_by_id(res.execution_id, &fetched);
    assert(found);
    assert(strcmp(fetched.execution_id, res.execution_id) == 0);
    assert(fetched.native_error_code == res.native_error_code);

    OperationalStats stats;
    database_get_stats(&stats);
    assert(stats.total_executions > 0);

    database_close();
    platform_delete_file("data/test_db.db");
    printf("[PASS] test_database_persistence\n");
}

int main(void) {
    printf("=== Running SysCallLab Backend Unit Tests ===\n");
    config_init_defaults();
    config_ensure_directories();
    platform_init();

    test_platform_info();
    test_missing_file_detection();
    test_file_write_and_read();
    test_invalid_descriptor();
    test_database_persistence();

    platform_cleanup();
    printf("=== All Backend Unit Tests Passed Successfully! ===\n");
    return 0;
}
