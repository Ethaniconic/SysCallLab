#ifndef SYSCALL_RECOVERY_H
#define SYSCALL_RECOVERY_H

#include "common.h"

typedef struct {
    char execution_id[MAX_ID_LENGTH];
    char strategy_name[64];
    bool user_confirmed;
    bool success;
    char details[MAX_ERROR_DESC];
} RecoveryExecutionResult;

bool recovery_execute(const char* execution_id, const char* strategy, bool confirmed, RecoveryExecutionResult* out_result);

#endif // SYSCALL_RECOVERY_H
