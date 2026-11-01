#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/database.h"
#include "../../include/logger.h"
#include "../../third_party/sqlite3/sqlite3.h"

static sqlite3* s_db = NULL;

bool database_init(const char* db_path) {
    if (s_db) return true;

    int rc = sqlite3_open(db_path, &s_db);
    if (rc != SQLITE_OK) {
        logger_log(LOG_LEVEL_ERROR, "Cannot open SQLite database: %s", sqlite3_errmsg(s_db));
        return false;
    }

    const char* schema = 
        "CREATE TABLE IF NOT EXISTS executions ("
        "  execution_id TEXT PRIMARY KEY,"
        "  timestamp TEXT,"
        "  operation TEXT,"
        "  native_api TEXT,"
        "  return_value INTEGER,"
        "  native_error_code INTEGER,"
        "  error_identifier TEXT,"
        "  error_category TEXT,"
        "  description TEXT,"
        "  possible_cause TEXT,"
        "  suggested_recovery TEXT,"
        "  target_path TEXT,"
        "  success INTEGER,"
        "  recovery_status TEXT,"
        "  recovery_notes TEXT,"
        "  elapsed_us INTEGER"
        ");"
        "CREATE TABLE IF NOT EXISTS settings ("
        "  key TEXT PRIMARY KEY,"
        "  value TEXT"
        ");";

    char* err_msg = NULL;
    rc = sqlite3_exec(s_db, schema, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        logger_log(LOG_LEVEL_ERROR, "Failed to initialize database schema: %s", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    logger_log(LOG_LEVEL_INFO, "Database initialized successfully at: %s", db_path);
    return true;
}

void database_close(void) {
    if (s_db) {
        sqlite3_close(s_db);
        s_db = NULL;
    }
}

bool database_insert_execution(const ExecutionResult* res) {
    if (!s_db || !res) return false;

    const char* sql = "INSERT INTO executions (execution_id, timestamp, operation, native_api, return_value, "
                      "native_error_code, error_identifier, error_category, description, possible_cause, "
                      "suggested_recovery, target_path, success, recovery_status, recovery_notes, elapsed_us) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        logger_log(LOG_LEVEL_ERROR, "Prepare failed: %s", sqlite3_errmsg(s_db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, res->execution_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, res->timestamp, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, res->operation, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, res->native_api, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, res->return_value);
    sqlite3_bind_int(stmt, 6, res->native_error_code);
    sqlite3_bind_text(stmt, 7, res->error_identifier, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, res->error_category_str, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, res->description, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, res->possible_cause, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 11, res->suggested_recovery, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 12, res->target_path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 13, res->success ? 1 : 0);
    sqlite3_bind_text(stmt, 14, recovery_status_to_string(res->recovery_status), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 15, res->recovery_notes, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 16, res->elapsed_us);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool database_update_recovery(const char* execution_id, RecoveryStatus status, const char* notes) {
    if (!s_db || !execution_id) return false;

    const char* sql = "UPDATE executions SET recovery_status = ?, recovery_notes = ? WHERE execution_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, recovery_status_to_string(status), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, notes ? notes : "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, execution_id, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

static void fill_execution_result_from_stmt(sqlite3_stmt* stmt, ExecutionResult* res) {
    memset(res, 0, sizeof(ExecutionResult));
    const char* id = (const char*)sqlite3_column_text(stmt, 0);
    if (id) strncpy(res->execution_id, id, sizeof(res->execution_id) - 1);

    const char* ts = (const char*)sqlite3_column_text(stmt, 1);
    if (ts) strncpy(res->timestamp, ts, sizeof(res->timestamp) - 1);

    const char* op = (const char*)sqlite3_column_text(stmt, 2);
    if (op) strncpy(res->operation, op, sizeof(res->operation) - 1);

    const char* api = (const char*)sqlite3_column_text(stmt, 3);
    if (api) strncpy(res->native_api, api, sizeof(res->native_api) - 1);

    res->return_value = sqlite3_column_int64(stmt, 4);
    res->native_error_code = sqlite3_column_int(stmt, 5);

    const char* err_id = (const char*)sqlite3_column_text(stmt, 6);
    if (err_id) strncpy(res->error_identifier, err_id, sizeof(res->error_identifier) - 1);

    const char* cat_str = (const char*)sqlite3_column_text(stmt, 7);
    if (cat_str) {
        strncpy(res->error_category_str, cat_str, sizeof(res->error_category_str) - 1);
        res->category = string_to_error_category(cat_str);
    }

    const char* desc = (const char*)sqlite3_column_text(stmt, 8);
    if (desc) strncpy(res->description, desc, sizeof(res->description) - 1);

    const char* cause = (const char*)sqlite3_column_text(stmt, 9);
    if (cause) strncpy(res->possible_cause, cause, sizeof(res->possible_cause) - 1);

    const char* rec = (const char*)sqlite3_column_text(stmt, 10);
    if (rec) strncpy(res->suggested_recovery, rec, sizeof(res->suggested_recovery) - 1);

    const char* path = (const char*)sqlite3_column_text(stmt, 11);
    if (path) strncpy(res->target_path, path, sizeof(res->target_path) - 1);

    res->success = (sqlite3_column_int(stmt, 12) == 1);

    const char* rec_st = (const char*)sqlite3_column_text(stmt, 13);
    if (rec_st) res->recovery_status = string_to_recovery_status(rec_st);

    const char* notes = (const char*)sqlite3_column_text(stmt, 14);
    if (notes) strncpy(res->recovery_notes, notes, sizeof(res->recovery_notes) - 1);

    res->elapsed_us = sqlite3_column_int64(stmt, 15);
}

bool database_get_execution_by_id(const char* execution_id, ExecutionResult* out_result) {
    if (!s_db || !execution_id || !out_result) return false;

    const char* sql = "SELECT execution_id, timestamp, operation, native_api, return_value, "
                      "native_error_code, error_identifier, error_category, description, possible_cause, "
                      "suggested_recovery, target_path, success, recovery_status, recovery_notes, elapsed_us "
                      "FROM executions WHERE execution_id = ? LIMIT 1;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, execution_id, -1, SQLITE_TRANSIENT);
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        fill_execution_result_from_stmt(stmt, out_result);
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

bool database_query_executions(const char* filter_op, const char* filter_status, int limit, int offset, ExecutionRowCallback cb, void* user_data) {
    if (!s_db || !cb) return false;

    char sql[512];
    snprintf(sql, sizeof(sql), 
        "SELECT execution_id, timestamp, operation, native_api, return_value, "
        "native_error_code, error_identifier, error_category, description, possible_cause, "
        "suggested_recovery, target_path, success, recovery_status, recovery_notes, elapsed_us "
        "FROM executions ORDER BY timestamp DESC LIMIT %d OFFSET %d;",
        limit > 0 ? limit : 50, offset >= 0 ? offset : 0);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ExecutionResult row;
        fill_execution_result_from_stmt(stmt, &row);
        cb(&row, user_data);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool database_get_stats(OperationalStats* stats) {
    if (!s_db || !stats) return false;
    memset(stats, 0, sizeof(OperationalStats));

    const char* sql = "SELECT "
                      "COUNT(*), "
                      "SUM(CASE WHEN success = 1 THEN 1 ELSE 0 END), "
                      "SUM(CASE WHEN success = 0 THEN 1 ELSE 0 END), "
                      "SUM(CASE WHEN recovery_status = 'RECOVERED' THEN 1 ELSE 0 END) "
                      "FROM executions;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        stats->total_executions = sqlite3_column_int(stmt, 0);
        stats->successful_executions = sqlite3_column_int(stmt, 1);
        stats->failed_executions = sqlite3_column_int(stmt, 2);
        stats->recovered_operations = sqlite3_column_int(stmt, 3);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool database_export_csv(const char* output_filepath) {
    if (!s_db || !output_filepath) return false;

    FILE* f = fopen(output_filepath, "w");
    if (!f) return false;

    fprintf(f, "ExecutionID,Timestamp,Operation,NativeAPI,ReturnValue,NativeErrorCode,ErrorCategory,Success,RecoveryStatus,ElapsedUS,TargetPath\n");

    const char* sql = "SELECT execution_id, timestamp, operation, native_api, return_value, native_error_code, error_category, success, recovery_status, elapsed_us, target_path FROM executions ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            fprintf(f, "\"%s\",\"%s\",\"%s\",\"%s\",%lld,%d,\"%s\",%s,\"%s\",%lld,\"%s\"\n",
                (const char*)sqlite3_column_text(stmt, 0),
                (const char*)sqlite3_column_text(stmt, 1),
                (const char*)sqlite3_column_text(stmt, 2),
                (const char*)sqlite3_column_text(stmt, 3),
                (long long)sqlite3_column_int64(stmt, 4),
                sqlite3_column_int(stmt, 5),
                (const char*)sqlite3_column_text(stmt, 6),
                sqlite3_column_int(stmt, 7) ? "true" : "false",
                (const char*)sqlite3_column_text(stmt, 8),
                (long long)sqlite3_column_int64(stmt, 9),
                (const char*)sqlite3_column_text(stmt, 10)
            );
        }
        sqlite3_finalize(stmt);
    }
    fclose(f);
    return true;
}
