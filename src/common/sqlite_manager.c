#include "common/sqlite_manager.h"
#include "model/task_info.h"
#include "basics.h"
#include <stdlib.h>

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
    free(sql);
    if (err != SQLITE_OK) {
        printf("prepare failed: %s\n", sqlite3_errmsg(db));
        return RESULT_FAILURE;
    }
    for (int i = 0; i < parameter_type_count; ++i) {
        switch (parameter_types[i]) {
        case 'c': {
            char arg = va_arg(args, int);
            char parameter_holder[2] = { arg };
            sqlite3_bind_text(*p_stmt, i + 1, parameter_holder, -1, NULL);
            break;
        }
        case 's': {
            sqlite3_bind_text(*p_stmt, i + 1, va_arg(args, char*), -1, NULL);
            break;
        }
        case 'd': {
            int arg = va_arg(args, int);
            sqlite3_bind_int64(*p_stmt, i + 1, arg);
            break;
        }
        case 'f': {
            sqlite3_bind_double(*p_stmt, i + 1, va_arg(args, double));
            break;
        }
        default:
            printf("Unsupported parameter type: %c.\n", parameter_types[i]);
            exit(-1);
        }
    }
    return RESULT_OK;
}

int QuerySingle(QuerySingleResult* query_single_result, char* sql_format, ...) {
    if (db) {
        int result;
        sqlite3_stmt* stmt;
        va_list args;
        va_start(args, sql_format);
        if (Sqlite3PrepareWithParameters(&stmt, sql_format, args) ==
            RESULT_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                query_single_result->data =
                    malloc(query_single_result->element_size);
                query_single_result->data_retriever(query_single_result->data,
                                                    stmt);
                result = RESULT_OK;
            } else {
                printf("execution failed: %s\n", sqlite3_errmsg(db));
                result = RESULT_FAILURE;
            }
            sqlite3_finalize(stmt);
        } else {
            result = RESULT_FAILURE;
        }
        va_end(args);
        return result;
    } else {
        return RESULT_FAILURE;
    }
}

int QueryMany(QueryManyResult* query_result, char* sql_format, ...) {
    if (db) {
        int result;
        sqlite3_stmt* stmt;
        va_list args;
        va_start(args, sql_format);
        if (Sqlite3PrepareWithParameters(&stmt, sql_format, args) ==
            RESULT_OK) {
            if (!query_result->array) {
                query_result->array = g_ptr_array_new_with_free_func(free);
            }
            while (1) {
                int sql_result = sqlite3_step(stmt);
                if (sql_result == SQLITE_ROW) {
                    void* data = malloc(query_result->element_size);
                    g_ptr_array_add(query_result->array, data);
                    query_result->data_retriever(data, stmt);
                } else if (sql_result == SQLITE_ERROR) {
                    printf("execution failed: %s\n", sqlite3_errmsg(db));
                    result = RESULT_FAILURE;
                    break;
                } else {
                    result = RESULT_OK;
                    break;
                }
            }
            sqlite3_finalize(stmt);
        } else {
            result = RESULT_FAILURE;
        }
        va_end(args);
        return result;
    } else {
        return RESULT_FAILURE;
    }
}

sqlite3_int64 GetLastInsertRowId() {
    return sqlite3_last_insert_rowid(db);
}

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
        return result;
    } else {
        return RESULT_FAILURE;
    }
}

void CloseDatabase() {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}
