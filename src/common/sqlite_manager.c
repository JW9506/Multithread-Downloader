#include "common/sqlite_manager.h"
#include "model/task_info.h"
#include "basics.h"

static sqlite3* db;

static int Sqlite3PrepareWithParameters(sqlite3_stmt** p_stmt,
                                        char const* sql_format, va_list args) {
    char* sql = strdup(sql_format);
    int sql_length = strlen(sql);
    char parameter_types[20];
    int parameter_type_count = 0;
    for (int i = 0; i < sql_length; ++i) {
        if (sql[i] == '$') {
            if (sql[i + 1] != '$') {
                parameter_types[parameter_type_count++] = sql[i + 1];
                sql[i] = '?';
            }
            sql[i + 1] = ' ';
        }
    }
    printf("DB Execute %s\n", sql);
    int err = sqlite3_prepare_v2(db, sql, -1, p_stmt, NULL);
    return RESULT_OK;
}

/* static void TaskInfoRetriever(TaskInfo* task_info, sqlite3_stmt* result) {
    task_info->id = sqlite3_column_int64(result, 0);
    task_info->url = strdup((char*)sqlite3_column_text(result, 1));
} */

int OpenDataBase(char* path) {
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        printf("Can't open database: %s\n", sqlite3_errmsg(db));
        CloseDatabase();
        return RESULT_FAILURE;
    }
    return RESULT_OK;
}

int ExecuteSql(char* sql_format, ...) {
    if (db) {
        int result;
        sqlite3_stmt* stmt;
        va_list args;
        va_start(args, sql_format);
        if (Sqlite3PrepareWithParameters(&stmt, sql_format, args) ==
            RESULT_OK) {
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                printf("execution failed: %s\n", sqlite3_errmsg(db));
                result = RESULT_FAILURE;
            } else {
                result = RESULT_OK;
            }
            sqlite3_finalize(stmt);
        } else {
            result = RESULT_FAILURE;
        }
        va_end(args);
    } else {
        return RESULT_FAILURE;
    }
}

void CloseDatabase() {
    if (db) { sqlite3_close(db); }
}
