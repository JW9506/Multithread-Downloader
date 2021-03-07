#include "ui/ui_main.h"
#include "utils/ui_utils.h"
#include "ui/ui_new_task.h"

static void SetupButtons(GtkBuilder* builder) {
    GtkButton* add_button =
        (GtkButton*)gtk_builder_get_object(builder, "add_button");
    g_signal_connect(add_button, "clicked", G_CALLBACK(OpenNewTaskDialog), NULL);
}
static void InitTaskList(GtkBuilder* builder) { }
static void DestroyTaskList() { }

static void GtkMain(int argc, char** argv) {
    gtk_init(&argc, &argv);
    GtkBuilder* builder = OpenWindowWithStyle("downloader.ui", "downloader.css", 1);
    if (!builder) { g_abort(); }
    SetupButtons(builder);
    InitTaskList(builder);
    printf("hello>>\n");
    gtk_main();
    printf("hello>><<\n");
    DestroyTaskList();
    g_object_unref(builder);
}

int RunDownloaderMain(int argc, char** argv) {
    GtkMain(argc, argv);
    return 0;
}
