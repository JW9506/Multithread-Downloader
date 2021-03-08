#pragma once
#include <sqlite3.h>
#include <gtk-3.0/gtk/gtk.h>

#define STATUS_REMOVED     0
#define STATUS_ERROR       1
#define STATUS_READY       2
#define STATUS_PAUSED      3
#define STATUS_DOWNLOADING 4
#define STATUS_COMPLETED   5

#define STATUS_ERROR_TEXT       "Error"
#define STATUS_READY_TEXT       "Ready"
#define STATUS_PAUSED_TEXT      "Paused"
#define STATUS_DOWNLOADING_TEXT "Downloading"
#define STATUS_COMPLETED_TEXT   "Completed"

#define INVALID_ID -1

typedef struct {
    gint64 id;
    char* filename;
    char* directory;
    char* url;
    gint64 size;
    guint64 progress;
    guint status;
    char* create_time;
    gboolean resume_support;
} TaskInfo;

int InsertTaskInfo(TaskInfo* task_info);
void UpdateTaskInfo(TaskInfo* task_info);
void DeleteTaskInfo(TaskInfo* task_info);
TaskInfo* FindTaskInfoById(sqlite3_int64 id);
GPtrArray* ListTaskInfos();
void DestroyTaskInfo(TaskInfo* task_info);
void TaskInfoDump(TaskInfo* task_info);
