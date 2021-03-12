#pragma once

#include <sqlite3.h>
#include <gtk/gtk.h>
#include <glib.h>

#define MAX_SQL_LENGTH

typedef void (*DataRetriever)(void*, sqlite3_stmt*);

typedef struct QuerySingleResult {
    size_t element_size;
    DataRetriever data_retriever;
    void* data;
} QuerySingleResult;

typedef struct QueryManyResult {
    size_t element_size;
    DataRetriever data_retriever;
    GPtrArray* array;
} QueryManyResult;

int QuerySingle(QuerySingleResult* query_single_result, char* sql_format, ...);
int QueryMany(QueryManyResult* query_result, char* sql_format, ...);
sqlite3_int64 GetLastInsertRowId();
void CloseDatabase();
int OpenDataBase(char* path);
int ExecuteSql(char* sql_format, ...);
