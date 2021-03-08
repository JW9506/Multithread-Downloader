#pragma once
#include "model/task_info.h"

typedef struct TaskListContext {
    GtkWindow* window;
    GtkTreeSelection* task_selection;
    GtkListStore* task_store;
    GSList* download_task_list;
    int downloading_task_size;
} TaskListContext;

void InitTaskList(GtkBuilder* builder);
void DestroyTaskList();
void AddTaskToList(TaskInfo* task_info);
void RemoveSelectedTask();
void ResumeSelectedTask();
void PauseSelectedTask();
