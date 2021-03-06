#include <stdio.h>
#include <string.h>
#include "model/task_info.h"

static void TestDatabase() {
    TaskInfo task_info = {
        .filename = "abc.txt",
        .url = "http://www.bennyhuo.com/test/abc.txt",
        .directory = "~Downloads",
        .size = 1024,
    };
    InsertTaskInfo(&task_info);
    GPtrArray* task = ListTaskInfos();
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
}

int main(int argc, char** argv) {
    printf("123\n");
    return 0;
}
