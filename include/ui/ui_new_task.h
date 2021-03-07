#pragma once
#include <gtk/gtk.h>

void CreateTask(char const* url, char const* directory);
void OpenNewTaskDialog(GtkWidget* widget, gpointer data);
