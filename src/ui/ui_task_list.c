#include "ui/ui_task_list.h"
#include "ui/ui_download_task.h"
#include "model/request_api.h"
#include "utils/ui_utils.h"
#include "utils/object_util.h"

static TaskListContext* context;

static DownloadTask* GetSelectedTask() {
    GtkTreeModel* task_store;
    GtkTreeIter iterator;
    if (gtk_tree_selection_get_selected(context->task_selection, &task_store,
                                        &iterator)) {
        GList* selected_rows = gtk_tree_selection_get_selected_rows(
            context->task_selection, &task_store);
        int selected_index =
            gtk_tree_path_get_indices(g_list_first(selected_rows)->data)[0];
        g_list_free_full(selected_rows, (GDestroyNotify)gtk_tree_path_free);
        printf("%d\n", selected_index);
        return g_slist_nth(context->download_task_list, selected_index)->data;
    }
    return NULL;
}

static void OnTaskListActivated(GtkTreeView* tree_view, GtkTreePath* path,
                                GtkTreeViewColumn* column, gpointer user_data) {
    DownloadTask* selected_download_task = GetSelectedTask();
    if (selected_download_task->task_info.status != STATUS_COMPLETED) {
        if (selected_download_task->task_info.status == STATUS_DOWNLOADING) {
            printf("pausing\n");
            PauseSelectedTask();
        } else {
            printf("resuming\n");
            ResumeSelectedTask();
        }
    }
}

// todo: check
static void CancelDownloadingAndDestroyTask(DownloadTask* download_task) {
    // 16-12 2:23 request_handler assigned value in UI thread, not io thread as
    // said
    if (download_task->request_handler != NULL &&
        download_task->task_info.status == STATUS_DOWNLOADING) {
        download_task->task_info.status = STATUS_REMOVED;
        CancelRequest(download_task->request_handler);
    } else {
        DestroyDownloadTask(download_task);
    }
}

void DestroyTaskList() {
    if (context) {
        g_slist_free_full(context->download_task_list,
                          (GDestroyNotify)CancelDownloadingAndDestroyTask);
        while (context->downloading_task_size) {
            while (gtk_events_pending()) {
                gtk_main_iteration();
                printf("waiting for task to stop: %d\n",
                       context->downloading_task_size);
                sleepms(100);
            }
            free(context);
            context = NULL;
            gtk_main_quit();
        }
    }
}

void InitTaskList(GtkBuilder* builder) {
    if (!context) {
        context = malloc(sizeof(TaskListContext));
        context->window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
        g_signal_connect_swapped(context->window, "destroy",
                                 G_CALLBACK(DestroyTaskList), NULL);
        context->task_store =
            GTK_LIST_STORE(gtk_builder_get_object(builder, "task_store"));
        context->task_selection = GTK_TREE_SELECTION(
            gtk_builder_get_object(builder, "task_selection"));
        GtkTreeView* task_list_view =
            GTK_TREE_VIEW(gtk_builder_get_object(builder, "task_tree"));
        // on row selected
        g_signal_connect(task_list_view, "row-activated",
                         G_CALLBACK(OnTaskListActivated), NULL);
        GPtrArray* task_infos = ListTaskInfos();
        context->download_task_list = NULL;
        context->downloading_task_size = 0;
        for (int i = 0; i < task_infos->len; ++i) {
            TaskInfo* task_info = g_ptr_array_index(task_infos, i);
            TaskInfoDump(task_info);
            context->download_task_list =
                g_slist_append(context->download_task_list,
                               CreateDownloadTask(context, task_info));
        }
        g_ptr_array_free(task_infos, TRUE);
    }
}
void AddTaskToList(TaskInfo* task_info) {
    context->download_task_list = g_slist_append(
        context->download_task_list, CreateDownloadTask(context, task_info));
}

void RemoveSelectedTask() {
    DownloadTask* selected_download_task = GetSelectedTask();
    if (selected_download_task) {
        if (ShowConfirmDialog(context->window,
                              "Do you want to remove this task?")) {
            context->download_task_list = g_slist_remove(
                context->download_task_list, selected_download_task);
            DeleteTaskInfo(&selected_download_task->task_info);
            CancelDownloadingAndDestroyTask(selected_download_task);
        }
    }
}

void ResumeSelectedTask() {
    DownloadTask* selected_download_task = GetSelectedTask();
    if (selected_download_task &&
        selected_download_task->task_info.status < STATUS_DOWNLOADING) {
        DownloadFile(selected_download_task,
                     &selected_download_task->request_handler,
                     &selected_download_task->task_info,
                     (OnRequestSuccess)OnDownloadCompleted,
                     (OnRequestError)OnDownloadFailed,
                     (OnRequestCancellation)OnDownloadCancelled,
                     (OnRequestProgress)OnProgressUpdated);
        UpdateDownloadTaskWithStatus(selected_download_task,
                                     STATUS_DOWNLOADING);
    }
}

void PauseSelectedTask() {
    DownloadTask* selected_download_task = GetSelectedTask();
    if (selected_download_task &&
        selected_download_task->task_info.status == STATUS_DOWNLOADING) {
        CancelRequest(selected_download_task->request_handler);
    }
}
