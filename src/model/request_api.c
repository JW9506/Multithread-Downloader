#include "model/request_api.h"
#include "common/http_manager.h"
#include "common/threadpool_manager.h"
#include "utils/object_util.h"

static void DownloadFileInternal(Request* request) {
    TaskInfo* task_info = (TaskInfo*)request->request_parameter;
    RequestContext* context =
        CreateRequestContext(request, task_info->url, task_info->directory);
    if (request->request_handler) {
        *request->request_handler = context;
    }
    context->header_only = 0;
    if (task_info->resume_support) {
        context->range_start = task_info->progress;
    }
    context->file_size = task_info->size;
    SendRequest(context);
    if (context->curl_code == CURLE_OK) {
        if (context->response_code >= 200 && context->response_code < 300) {
            request->success_callback(request->receiver, NULL);
        } else {
            request->error_callback(request->receiver, context->response_code,
                                    NULL);
        }
    } else if (context->curl_code == CURLE_ABORTED_BY_CALLBACK) {
        request->cancel_callback(request->receiver);
    } else {
        request->error_callback(request->receiver, context->curl_code,
                                curl_easy_strerror(context->curl_code));
    }
    if (request->request_handler) { *request->request_handler = NULL; }
    free(request);
    DestroyRequestContext(context);
    printf("end>>>\n");
}

static void GetFileInfoInternal(Request* request) {
    TaskInfoParameter* parameter = request->request_parameter;
    RequestContext* context =
        CreateRequestContext(request, parameter->url, parameter->directory);
    context->header_only = 1;
    context->range_start = 0;
    SendRequest(context);
    if (context->curl_code == CURLE_OK) {
        CREATE_OBJECT_CLEANED(TaskInfo, task_info);
        task_info->url = parameter->url;
        task_info->directory = parameter->directory;
        task_info->filename = strdup(context->output_filename);
        task_info->size = context->file_size;
        task_info->resume_support = context->response_code == 206;
        task_info->status = STATUS_READY;
        request->success_callback(request->receiver, task_info);
    } else {
        request->error_callback(request->receiver, context->curl_code,
                                curl_easy_strerror(context->curl_code));
    }
    free(parameter);
    free(request);
    DestroyRequestContext(context);
}

void GetTaskInfo(char const* url, char const* directory,
                 OnRequestSuccess success_callback,
                 OnRequestError error_callback) {
    CREATE_OBJECT_CLEANED(TaskInfoParameter, parameter);
    parameter->url = g_strstrip(strdup(url));
    parameter->directory = g_strstrip(strdup(directory));
    CREATE_OBJECT_CLEANED(Request, request);
    request->request_parameter = parameter;
    request->error_callback = error_callback;
    request->success_callback = success_callback;
    ThreadPoolExecute(RUNNABLE(GetFileInfoInternal, request));
}

void DownloadFile(void* receiver, void** request_handler, TaskInfo* task_info,
                  OnRequestSuccess success_callback,
                  OnRequestError error_callback,
                  OnRequestCancellation cancel_callback,
                  OnRequestProgress progress_callback) {
    CREATE_OBJECT_CLEANED(Request, request);
    request->receiver = receiver;
    request->request_handler = request_handler;
    request->request_parameter = task_info;

    request->success_callback = success_callback;
    request->error_callback = error_callback;
    request->cancel_callback = cancel_callback;
    request->progress_callback = progress_callback;

    ThreadPoolExecute(RUNNABLE(DownloadFileInternal, request));
    // DownloadFileInternal(request);
}

void CancelRequest(void* request_handler) {
    RequestContext* context = (RequestContext*)request_handler;
    printf("hello>>\n");
    if (context) {
        printf("world>>\n");
        context->is_running = 0;
    }
}
