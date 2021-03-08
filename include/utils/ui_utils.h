#pragma once
#include <gtk/gtk.h>

GtkBuilder* OpenWindowWithStyle(char* layout_path, char* style_path,
                                int quit_on_destroy);

void ShowAlertDialog(GtkWindow* window, const char* message);

gboolean ShowConfirmDialog(GtkWindow* window, char const* message);
