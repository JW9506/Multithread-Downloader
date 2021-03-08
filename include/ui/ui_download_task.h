#pragma once
#include "ui/ui_task_list.h"

enum {
    COLUMN_FILENAME,
    COLUMN_SIZE,
    COLUMN_PROGRESS,
    COLUMN_STATUS,
    COLUMN_ESTIMATE_TIME,
    COLUMN_TRANSFER_TIME,
    COLUMN_CREATE_TIME,
    COLUMN_ID,
    N_COLUMN,
};

typedef struct DownloadTask {
    TaskListContext* task_list_context;
    TaskInfo task_info;
    void* request_handler;
    GtkTreeRowReference* row_reference;
    uint64_t last_progress;
    uint64_t last_update_time;
} DownloadTask;

DownloadTask* CreateDownloadTask(TaskListContext* task_list_context,
                                 TaskInfo* task_info);

void DestroyDownloadTask(DownloadTask* download_task);

void RemoveDownloadTaskFromList(DownloadTask* download_task);

void UpdateDownloadTaskWithStatus(DownloadTask* download_task, int status);

int OnDownloadCompleted(DownloadTask* task);

int OnDownloadCancelled(DownloadTask* task);

int OnDownloadFailed(DownloadTask* task);

int OnProgressUpdated(DownloadTask* task, uint64_t current_types,
                      uint64_t total_bytes);
