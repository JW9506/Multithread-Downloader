#pragma once
#include <sqlite3.h>
#include <gtk-3.0/gtk/gtk.h>

#define STATUS_COMPLETED 1

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
void DestroyTaskInfo(TaskInfo** task_info);
void TaskInfoDump(TaskInfo* task_info);
