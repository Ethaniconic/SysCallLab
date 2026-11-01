#include "../../include/common.h"

const char* error_category_to_string(ErrorCategory category) {
    switch (category) {
        case ERR_CAT_NONE: return "NONE";
        case ERR_CAT_FILE_NOT_FOUND: return "FILE_NOT_FOUND";
        case ERR_CAT_PERMISSION_DENIED: return "PERMISSION_DENIED";
        case ERR_CAT_INVALID_DESCRIPTOR: return "INVALID_DESCRIPTOR";
        case ERR_CAT_RESOURCE_EXHAUSTION: return "RESOURCE_EXHAUSTION";
        case ERR_CAT_INTERRUPTED_OPERATION: return "INTERRUPTED_OPERATION";
        case ERR_CAT_PROCESS_CREATION_FAILURE: return "PROCESS_CREATION_FAILURE";
        case ERR_CAT_EXECUTION_FAILURE: return "EXECUTION_FAILURE";
        case ERR_CAT_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case ERR_CAT_MEMORY_ALLOCATION_FAILURE: return "MEMORY_ALLOCATION_FAILURE";
        case ERR_CAT_UNKNOWN_ERROR:
        default: return "UNKNOWN_ERROR";
    }
}

ErrorCategory string_to_error_category(const char* str) {
    if (!str) return ERR_CAT_UNKNOWN_ERROR;
    if (strcmp(str, "NONE") == 0) return ERR_CAT_NONE;
    if (strcmp(str, "FILE_NOT_FOUND") == 0) return ERR_CAT_FILE_NOT_FOUND;
    if (strcmp(str, "PERMISSION_DENIED") == 0) return ERR_CAT_PERMISSION_DENIED;
    if (strcmp(str, "INVALID_DESCRIPTOR") == 0) return ERR_CAT_INVALID_DESCRIPTOR;
    if (strcmp(str, "RESOURCE_EXHAUSTION") == 0) return ERR_CAT_RESOURCE_EXHAUSTION;
    if (strcmp(str, "INTERRUPTED_OPERATION") == 0) return ERR_CAT_INTERRUPTED_OPERATION;
    if (strcmp(str, "PROCESS_CREATION_FAILURE") == 0) return ERR_CAT_PROCESS_CREATION_FAILURE;
    if (strcmp(str, "EXECUTION_FAILURE") == 0) return ERR_CAT_EXECUTION_FAILURE;
    if (strcmp(str, "INVALID_ARGUMENT") == 0) return ERR_CAT_INVALID_ARGUMENT;
    if (strcmp(str, "MEMORY_ALLOCATION_FAILURE") == 0) return ERR_CAT_MEMORY_ALLOCATION_FAILURE;
    return ERR_CAT_UNKNOWN_ERROR;
}

const char* recovery_status_to_string(RecoveryStatus status) {
    switch (status) {
        case RECOVERY_NOT_ATTEMPTED: return "NOT_ATTEMPTED";
        case RECOVERY_ATTEMPTED_SUCCESS: return "RECOVERED";
        case RECOVERY_ATTEMPTED_FAILED: return "RECOVERY_FAILED";
        case RECOVERY_NOT_APPLICABLE: return "NOT_APPLICABLE";
        default: return "NOT_ATTEMPTED";
    }
}

RecoveryStatus string_to_recovery_status(const char* str) {
    if (!str) return RECOVERY_NOT_ATTEMPTED;
    if (strcmp(str, "RECOVERED") == 0) return RECOVERY_ATTEMPTED_SUCCESS;
    if (strcmp(str, "RECOVERY_FAILED") == 0) return RECOVERY_ATTEMPTED_FAILED;
    if (strcmp(str, "NOT_APPLICABLE") == 0) return RECOVERY_NOT_APPLICABLE;
    return RECOVERY_NOT_ATTEMPTED;
}
