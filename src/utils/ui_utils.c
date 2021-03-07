#include "utils/ui_utils.h"

GtkBuilder* OpenWindowWithStyle(char* layout_path, char* style_path,
                                int quit_on_destroy) {
    GError* error = NULL;
    GtkCssProvider* css_provider = gtk_css_provider_new();
    if (gtk_css_provider_load_from_path(css_provider, style_path, &error) ==
        0) {
        g_printerr("Error loading css file: %s\n", error->message);
        g_clear_error(&error);
        return NULL;
    }
    GtkBuilder* builder = gtk_builder_new();
    if (gtk_builder_add_from_file(builder, layout_path, &error) == 0) {
        g_printerr("Error loading builder file: %s\n", error->message);
        g_clear_error(&error);
        return NULL;
    }
    GtkWindow* window = (GtkWindow*)gtk_builder_get_object(builder, "window");
    gtk_style_context_add_provider_for_screen(gtk_window_get_screen(window),
                                              (GtkStyleProvider*)css_provider,
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);

    if (quit_on_destroy) {
        g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    }
    printf("world\n");

/*     gtk_style_context_remove_provider_for_screen(
        gtk_window_get_screen(window), (GtkStyleProvider*)css_provider);
    g_object_unref(css_provider); */
    return builder;
}
