#include "ui/ui_new_task.h"
#include "utils/ui_utils.h"
#include "utils/file_utils.h"
#include "stdlib.h"

#define URL_PATTERN                                                            \
    "https:\\/\\/(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]{1,256}\\.[a-zA-Z0-9()]{}"

static struct NewTaskDialogContext {
    GtkBuilder* builder;
    GtkDialog* dialog;
    GtkEntry* url_entry;
    GtkLabel* url_error_label;
    GtkFileChooserButton* directory_choose_button;
    GtkLabel* directory_error_label;
    GtkSpinner* ok_spinner;
    GtkButton* ok_button;
    GtkButton* cancel_button;
} * context;

static gboolean OnTaskCreatedSuccessful(char const* message) { }

static gboolean OnTaskCreatedFailed(char const* message) { }

static void OnGetTaskInfoSuccessful(void* receiver, void* data) { }

static void OnGetTaskInfoFailed(void* receiver, int code, char const* message) {
}

void CreateTask(char const* url, char const* directory) { }

static void OnConfirmCreateTask(GtkWidget* widget, gpointer data) { }

static void OnDialogDestroyed(GtkWidget* widget, gpointer data) {
    free(context);
    context = NULL;
    printf("Destroyed new task dialog...\n");
}

void OpenNewTaskDialog(GtkWidget* widget, gpointer data) {
    GtkBuilder* builder =
        OpenWindowWithStyle("downloader_new_task.ui", "downloader.css", 0);
    if (!builder) { g_abort(); }
    context = (struct NewTaskDialogContext*)malloc(
        sizeof(struct NewTaskDialogContext));
    context->builder = builder;

    context->dialog = (GtkDialog*)gtk_builder_get_object(builder, "window");
    g_signal_connect(context->dialog, "destroy", G_CALLBACK(OnDialogDestroyed),
                     NULL);

    context->ok_button =
        (GtkButton*)gtk_builder_get_object(builder, "ok_button");
    g_signal_connect_swapped(context->ok_button, "clicked",
                             G_CALLBACK(OnConfirmCreateTask), NULL);

    context->cancel_button =
        (GtkButton*)gtk_builder_get_object(builder, "cancel_button");

    context->url_entry =
        (GtkEntry*)gtk_builder_get_object(builder, "url_entry");
    context->url_error_label =
        (GtkLabel*)gtk_builder_get_object(builder, "url_error_label");
    char* copied_text = gtk_clipboard_wait_for_text(
        gtk_clipboard_get_default(gdk_display_get_default()));
    if (copied_text) {
        gtk_entry_set_text(context->url_entry, copied_text);
        printf("Copied: %s\n", copied_text);
    } else {
        printf("Nothing was copied\n");
    }
    context->directory_choose_button =
        (GtkFileChooserButton*)gtk_builder_get_object(
            builder, "directory_choose_button");

    context->directory_error_label =
        (GtkLabel*)gtk_builder_get_object(builder, "directory_error_button");

    char const* home_dir = g_get_home_dir();
    char default_directory[strlen(home_dir) + 10];
    JoinPath(default_directory, home_dir, "Downloads");
    gtk_file_chooser_set_filename(
        (GtkFileChooser*)context->directory_choose_button, default_directory);
    context->ok_spinner =
        (GtkSpinner*)gtk_builder_get_object(builder, "ok_spinner");

    gint response = gtk_dialog_run(context->dialog);
    printf("response: %d\n", response);
    gtk_widget_destroy(GTK_WIDGET(context->dialog));
}
