#pragma once

typedef struct {
    gint64 id;
    char* filename;
    char* directory;
    char* url;
    gint64 size;
    guint64 progress;
    guint status;
    char* creat_time;
    gboolean resume_support;
} TaskInfo;

void TaskInfoDump(TaskInfo* task_info);

void DestroyTaskInfo(TaskInfo** task_info);

void InsertTaskInfo(TaskInfo* task_info);

void UpdateTaskInfo(TaskInfo* task_info);
