#pragma once
#include "common/http_common.h"
#include "model/task_info.h"

typedef struct TaskInfoParameter {
    char* url;
    char* directory;
} TaskInfoParameter;

void GetTaskInfo(char const* url, char const* directory,
                 OnRequestSuccess success_callback,
                 OnRequestError error_callback);

void DownloadFile(void* receiver, void** request_handler, TaskInfo* task_info,
                  OnRequestSuccess success_callback,
                  OnRequestError error_callback,
                  OnRequestCancellation cancel_callback,
                  OnRequestProgress progress_callback);

void CancelRequest(void* request_handler);
