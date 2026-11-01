#ifndef SYSCALL_DATABASE_H
#define SYSCALL_DATABASE_H

#include "common.h"

typedef struct {
    int total_executions;
    int successful_executions;
    int failed_executions;
    int recovered_operations;
} OperationalStats;

bool database_init(const char* db_path);
void database_close(void);

bool database_insert_execution(const ExecutionResult* result);
bool database_update_recovery(const char* execution_id, RecoveryStatus status, const char* notes);
bool database_get_execution_by_id(const char* execution_id, ExecutionResult* out_result);

typedef void (*ExecutionRowCallback)(const ExecutionResult* res, void* user_data);
bool database_query_executions(const char* filter_op, const char* filter_status, int limit, int offset, ExecutionRowCallback cb, void* user_data);

bool database_get_stats(OperationalStats* stats);
bool database_export_csv(const char* output_filepath);

#endif // SYSCALL_DATABASE_H
