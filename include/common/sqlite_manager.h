#pragma once

#include <sqlite3.h>
#include <gtk-3.0/gtk/gtk.h>
#include <glib-2.0/glib.h>

#define MAX_SQL_LENGTH

typedef void (*DataRetriever)(void*, sqlite3_stmt*);

typedef struct QuerySingleResult {
    size_t element_size;
    DataRetriever data_retriever;
    void* data
} QuerySingleResult;

typedef struct QueryManyResult {
    size_t element_size;
    DataRetriever data_retriever;
    GPtrArray* array;
} QueryManyResult;

void CloseDatabase();
int OpenDataBase(char* path);
