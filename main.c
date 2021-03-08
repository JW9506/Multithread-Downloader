#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/sqlite_manager.h"
#include "model/database_common.h"
#include "model/task_info.h"
#include "common/http_manager.h"
#include "common/http_common.h"
#include "model/request_api.h"
#include "common/threadpool_manager.h"
#include "utils/object_util.h"
#include "ui/ui_main.h"
#include "basics.h"

static void TestDatabase() {
    TaskInfo task_info = {
        .filename = "abc.txt",
        .url = "http://www.google.com/test/abc.txt",
        .directory = "~/Downloads",
        .size = 1024,
    };
    InsertTaskInfo(&task_info);
    GPtrArray* tasks = ListTaskInfos();
    if (!tasks) {
        printf("List from DB error\n");
        exit(1);
    }
    printf("Found %d task(s)\n", tasks->len);
    for (int i = 0; i < tasks->len; ++i) {
        TaskInfo* task = g_ptr_array_index(tasks, i);
        TaskInfoDump(task);
    }

    g_ptr_array_free(tasks, TRUE);
    task_info.progress = 100;
    task_info.status = STATUS_COMPLETED;
    UpdateTaskInfo(&task_info);
    int last_task_id = GetLastInsertRowId();
    printf("%d\n", last_task_id);
    TaskInfo* last_inserted_task = FindTaskInfoById(last_task_id);
    if (last_inserted_task) {
        TaskInfoDump(last_inserted_task);
        // DeleteTaskInfo(last_inserted_task);
        DestroyTaskInfo(last_inserted_task);
    }
}

void OnProgress(void* receiver, uint64_t current, uint64_t total) {
    printf(">>>> OnProgress %lld/%lld\n", current, total);
}
void OnError() { printf(">>>> OnError\n"); }
void OnSuccess() { printf(">>>> OnSuccess\n"); }
void OnCancellation() { printf(">>>> OnCancellation\n"); }

static void TestHttp() {
    Request request = { .progress_callback = OnProgress,
                        .error_callback = OnError,
                        .success_callback = OnSuccess };
    RequestContext* context = CreateRequestContext(
        &request,
        "https://dl.bintray.com/conan-community/conan/conan/OpenSSL/1.0.2s/"
        "stable/0/export/conanfile.py",
        "./");
    // "http://5b0988e595225.cdn.sohucs.com/images/20190925/"
    // "86ce0e8c7ea045e3bc7d25b8a008b008.jpeg",
    SendRequest(context);
    if (context->curl_code == CURLE_OK) {
        request.success_callback(request.receiver, "OK");
    } else {
        printf(">>>curl_code %d\n", context->curl_code);
        request.error_callback(request.receiver, context->curl_code,
                               curl_easy_strerror(context->curl_code));
    }
    DestroyRequestContext(context);
}

static void TestDownloadFile() {
    CREATE_OBJECT_CLEANED(TaskInfo, task_info);
    task_info->filename = "lyf.jpeg";
    task_info->directory = "./";
    task_info->url = "http://txt.janpn.com/to/txt/171/171182.txt";
    task_info->resume_support = 1;
    task_info->size = 1000000;
    void* handler = malloc(sizeof(void*));
    DownloadFile(NULL, &handler, task_info, OnSuccess, OnError, OnCancellation,
                 OnProgress);

    sleepms(1000);
    // CancelRequest(handler);
    free(task_info);
    free(handler);
}

static void OnGetTaskInfoSuccess(void* receiver, void* data) {
    printf("success:\n");
    TaskInfoDump((TaskInfo*)data);
    free(data);
}

static void TestTaskInfo() {
    GetTaskInfo("http://5b0988e595225.cdn.sohucs.com/images/20190925/"
                "86ce0e8c7ea045e3bc7d25b8a008b008.jpeg",
                ".", OnGetTaskInfoSuccess, OnError);
}

int main(int argc, char** argv) {
    if (!is_ok(OpenDataBase("downloader.db"))) {
        return 1;
    }
    InitTables();
    // TestDatabase();
    // TestHttp();
    InitThreadPool();
    RunDownloaderMain(argc, argv);
    // TestTaskInfo();
    // TestDownloadFile();
    // sleepms(5000);
    // while (1) { }
    DestroyThreadPool();
    return 0;
}
