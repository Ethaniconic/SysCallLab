cmake_minimum_required(VERSION 3.10)
project(SysCallLab C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(
    backend/include
    backend/third_party/sqlite3
    backend/third_party/cJSON
)

# Common sources
set(COMMON_SOURCES
    backend/src/core/config.c
    backend/src/core/syscall_runner.c
    backend/src/core/error_analyzer.c
    backend/src/core/recovery.c
    backend/src/core/test_runner.c
    backend/src/server/http_server.c
    backend/src/server/api_routes.c
    backend/src/server/json_handler.c
    backend/src/storage/database.c
    backend/src/storage/logger.c
    backend/src/utils/time_utils.c
    backend/src/utils/string_utils.c
    backend/src/utils/validation.c
    backend/third_party/sqlite3/sqlite3.c
    backend/third_party/cJSON/cJSON.c
)

# Platform selection
if(WIN32)
    set(PLATFORM_SOURCES
        backend/src/platform/windows/file_ops.c
        backend/src/platform/windows/process_ops.c
        backend/src/platform/windows/error_mapping.c
    )
    set(PLATFORM_LIBS ws2_32)
elseif(APPLE)
    set(PLATFORM_SOURCES
        backend/src/platform/macos/file_ops.c
        backend/src/platform/macos/process_ops.c
        backend/src/platform/macos/error_mapping.c
    )
    set(PLATFORM_LIBS pthread)
else()
    set(PLATFORM_SOURCES
        backend/src/platform/linux/file_ops.c
        backend/src/platform/linux/process_ops.c
        backend/src/platform/linux/error_mapping.c
    )
    set(PLATFORM_LIBS pthread dl)
endif()

# Main Application Executable
add_executable(SysCallLab
    backend/src/main.c
    ${COMMON_SOURCES}
    ${PLATFORM_SOURCES}
)
target_link_libraries(SysCallLab ${PLATFORM_LIBS})

# Helper Executables for Controlled Process Testing
add_executable(test_child helper/test_child.c)
add_executable(test_executable helper/test_executable.c)

# Output directory
set_target_properties(SysCallLab test_child test_executable PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Testing
enable_testing()

add_executable(test_backend_unit
    backend/tests/test_backend.c
    ${COMMON_SOURCES}
    ${PLATFORM_SOURCES}
)
target_link_libraries(test_backend_unit ${PLATFORM_LIBS})
set_target_properties(test_backend_unit PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
add_test(NAME BackendUnitTests COMMAND test_backend_unit)
