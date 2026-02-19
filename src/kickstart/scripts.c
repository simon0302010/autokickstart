#include <gtk/gtk.h>

void new_script_window_with_title(const char *title) {
    GtkWidget *window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);
    gtk_window_present(GTK_WINDOW(window));
}