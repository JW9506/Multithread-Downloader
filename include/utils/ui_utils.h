#pragma once
#include <gtk/gtk.h>

GtkBuilder* OpenWindowWithStyle(char* layout_path, char* style_path,
                                int quit_on_destroy);
