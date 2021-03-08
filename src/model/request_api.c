#include "model/request_api.h"
#include "common/http_manager.h"
#include "common/threadpool_manager.h"
#include "utils/object_util.h"
#include <gtk/gtk.h>

static void DownloadFileInternal(Request* request) {
    TaskInfo* task_info = (TaskInfo*)request->request_parameter;

    RequestContext* context =
        CreateRequestContext(request, task_info->url, task_info->directory);

    if (request->request_handler) { *request->request_handler = context; }
    context->header_only = 0;
    if (task_info->resume_support) {
        context->range_start = task_info->progress;
    }
    context->file_size = task_info->size;
    SendRequest(context);
    if (context->curl_code == CURLE_OK) {
        if (context->response_code >= 200 && context->response_code < 300) {
            gdk_threads_add_idle((GSourceFunc)request->success_callback,
                                 (gpointer)request->receiver);
        } else {
            printf("curl http: %d\n", context->response_code);
            gdk_threads_add_idle((GSourceFunc)request->error_callback,
                                 (gpointer)request->receiver);
        }
    } else if (context->curl_code == CURLE_ABORTED_BY_CALLBACK) {
        gdk_threads_add_idle((GSourceFunc)request->cancel_callback,
                             (gpointer)request->receiver);
    } else {
        gdk_threads_add_idle((GSourceFunc)request->error_callback,
                             (gpointer)request->receiver); // 16-11, 16:47
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
    if (context) { context->is_running = 0; }
}
