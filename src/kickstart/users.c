#include <gtk/gtk.h>

#include "../utils/utils.h"
#include "../globals.h"

void new_users_window() {
    GtkWidget *window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "Manage Users");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(main_box, 15);
    gtk_widget_set_margin_end(main_box, 15);
    gtk_widget_set_margin_top(main_box, 15);
    gtk_widget_set_margin_bottom(main_box, 15);

    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // users label
    GtkWidget *users_label = create_label("Users");
    gtk_widget_set_halign(users_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_box), users_label);

    // users box
    GtkWidget *users_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    // scrollable box
    GtkWidget *users_box_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(users_box_scroll), TRUE);
    //gtk_widget_set_hexpand(code_scroll, TRUE);
    gtk_widget_set_vexpand(users_box_scroll, TRUE);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(users_box_scroll), users_box);
    gtk_box_append(GTK_BOX(main_box), users_box_scroll);

    // add user button
    GtkWidget *add_user_btn = gtk_button_new_with_label("Add User");
    gtk_box_append(GTK_BOX(main_box), add_user_btn);

    gtk_window_present(GTK_WINDOW(window));
}

void setup_users_window_items() {
    options.users.list = NULL;
    options.users.count = 0;
}