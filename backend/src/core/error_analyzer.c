#include <stdio.h>
#include <string.h>
#include "../../include/error_analyzer.h"
#include "../../include/platform.h"

void error_analyzer_enrich(ExecutionResult* result) {
    if (!result) return;

    if (result->native_error_code == 0 && result->success) {
        result->category = ERR_CAT_NONE;
        strncpy(result->error_category_str, "NONE", sizeof(result->error_category_str) - 1);
        strncpy(result->possible_cause, "The operation was authorized and executed cleanly by the operating system kernel.", sizeof(result->possible_cause) - 1);
        strncpy(result->suggested_recovery, "No error recovery needed.", sizeof(result->suggested_recovery) - 1);
        return;
    }

    result->category = platform_categorize_error(result->native_error_code);
    strncpy(result->error_category_str, error_category_to_string(result->category), sizeof(result->error_category_str) - 1);
    strncpy(result->error_identifier, platform_get_error_identifier(result->native_error_code), sizeof(result->error_identifier) - 1);

    platform_format_error(result->native_error_code, result->description, sizeof(result->description));

    switch (result->category) {
        case ERR_CAT_FILE_NOT_FOUND:
            strncpy(result->possible_cause, 
                    "The specified file or directory path does not exist in the filesystem, or the path syntax is invalid.", 
                    sizeof(result->possible_cause) - 1);
            strncpy(result->suggested_recovery, 
                    "Verify file existence, create the missing file in the designated safe test sandbox, or verify user file path.", 
                    sizeof(result->suggested_recovery) - 1);
            break;
        case ERR_CAT_PERMISSION_DENIED:
            strncpy(result->possible_cause, 
                    "Process lacks required read/write permissions for the target object, or file is locked exclusively by another handle.", 
                    sizeof(result->possible_cause) - 1);
            strncpy(result->suggested_recovery, 
                    "Adjust file attributes to writable mode, wait for concurrent handles to close, or check DACLs.", 
                    sizeof(result->suggested_recovery) - 1);
            break;
        case ERR_CAT_INVALID_DESCRIPTOR:
            strncpy(result->possible_cause, 
                    "The file handle or descriptor passed to the kernel API is closed, uninitialized, or outside the process handle table.", 
                    sizeof(result->possible_cause) - 1);
            strncpy(result->suggested_recovery, 
                    "Acquire a valid OS handle using CreateFile/open before performing I/O operations and avoid double-closing.", 
                    sizeof(result->suggested_recovery) - 1);
            break;
        case ERR_CAT_RESOURCE_EXHAUSTION:
            strncpy(result->possible_cause, 
                    "System heap, paging file, or non-paged kernel pool was exhausted when satisfying the allocation request.", 
                    sizeof(result->possible_cause) - 1);
            strncpy(result->suggested_recovery, 
                    "Release unreferenced buffers, close idle descriptors, and retry with smaller allocation chunks.", 
                    sizeof(result->suggested_recovery) - 1);
            break;
        case ERR_CAT_INTERRUPTED_OPERATION:
            strncpy(result->possible_cause, 
                    "Kernel I/O was preempted or cancelled by an asynchronous I/O completion routine, signal, or cancellation token.", 
                    sizeof(result->possible_cause) - 1);
            strncpy(result->suggested_recovery, 
                    "Restart the system call with remaining byte offset, or resume I/O loop upon EINTR.", 
                    sizeof(result->suggested_recovery) - 1);
            break;
        case ERR_CAT_PROCESS_CREATION_FAILURE:
            strncpy(result->possible_cause, 
                    "The executable image could not be loaded into memory, path was missing, or binary architecture is incompatible.", 
                    sizeof(result->possible_cause) - 1);
            strncpy(result->suggested_recovery, 
                    "Provide a valid absolute path to an authentic executable binary and verify PE/ELF header compatibility.", 
                    sizeof(result->suggested_recovery) - 1);
            break;
        case ERR_CAT_INVALID_ARGUMENT:
            strncpy(result->possible_cause, 
                    "One or more parameters supplied to the system API did not conform to kernel boundary checks.", 
                    sizeof(result->possible_cause) - 1);
            strncpy(result->suggested_recovery, 
                    "Ensure buffers, length parameters, and flag bitmasks conform strictly to OS API specifications.", 
                    sizeof(result->suggested_recovery) - 1);
            break;
        default:
            strncpy(result->possible_cause, 
                    "An unexpected OS-level anomaly or unclassified status code was returned by the kernel.", 
                    sizeof(result->possible_cause) - 1);
            strncpy(result->suggested_recovery, 
                    "Inspect native OS error logs and verify system-level environmental prerequisites.", 
                    sizeof(result->suggested_recovery) - 1);
            break;
    }
}
