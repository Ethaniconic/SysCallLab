#ifndef SYSCALL_TEST_RUNNER_H
#define SYSCALL_TEST_RUNNER_H

#include "common.h"

typedef struct {
    char test_id[64];
    char name[128];
    char category[64];
    char operation[64];
    char expected_outcome[128];
    char actual_outcome[128];
    int32_t expected_error_code;
    int32_t actual_error_code;
    bool passed;
    char details[256];
} TestCaseResult;

typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
    TestCaseResult results[32];
} TestSuiteSummary;

bool test_runner_run_suite(const char* category, TestSuiteSummary* summary);

#endif // SYSCALL_TEST_RUNNER_H
