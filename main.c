#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/sqlite_manager.h"
#include "model/database_common.h"
#include "model/task_info.h"

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
        DestroyTaskInfo(&last_inserted_task);
    }
}

int main(int argc, char** argv) {
    OpenDataBase("downloader.db");
    InitTables();
    TestDatabase();
    return 0;
}
