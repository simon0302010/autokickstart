#include <gtk/gtk.h>

void open_package_management(GtkWidget *open_management_button, gpointer user_data) {
    GtkWidget *pkg_window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(pkg_window), "Manage Packages");
    gtk_window_set_default_size(GTK_WINDOW(pkg_window), 300, 500);

    GtkWidget *placeholder_label = gtk_label_new("Coming soon");
    gtk_window_set_child(GTK_WINDOW(pkg_window), placeholder_label);

    gtk_window_present(GTK_WINDOW(pkg_window));
}