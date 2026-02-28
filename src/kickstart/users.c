#include <gtk/gtk.h>
#include <stdlib.h>

#include "utils/utils.h"
#include "globals.h"

GtkWidget *users_box;

User *create_user() {
    User *user = malloc(sizeof(User));

    user->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_margin_start(user->box, 5);
    gtk_widget_set_margin_end(user->box, 5);
    gtk_widget_set_margin_top(user->box, 5);

    user->username = gtk_entry_new();
    gtk_widget_set_tooltip_text(user->username, "Provides the name of the user. This option is required.");
    gtk_entry_set_placeholder_text(GTK_ENTRY(user->username), "Username");
    gtk_box_append(GTK_BOX(user->box), user->username);

    user->password = gtk_password_entry_new();
    gtk_widget_set_tooltip_text(user->password, "The new user’s password. If no password is provided, the account will be locked.");
    GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(user->password), "placeholder-text");
    if (pspec) {
        g_object_set(user->password, "placeholder-text", "Password", NULL);
    }
    gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(user->password), TRUE);

    gtk_box_append(GTK_BOX(user->box), user->password);

    gtk_box_append(GTK_BOX(user->box), create_label("   Groups:"));
    user->groups = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(user->groups), "group1, group2, group3");
    gtk_widget_set_tooltip_text(user->groups, "In addition to the default group, a comma separated list of group names the user should belong to. The groups must exist before the user account is created.");
    gtk_box_append(GTK_BOX(user->box), user->groups);

    gtk_box_append(GTK_BOX(user->box), create_label("   Gecos:"));
    user->gecos = gtk_entry_new();
    gtk_widget_set_tooltip_text(user->gecos, "Provides the GECOS information for the user. This is a string of various system-specific fields separated by a comma. It is frequently used to specify the user’s full name, office number, etc. See the passwd(5) man page for more details.");
    gtk_box_append(GTK_BOX(user->box), user->gecos);

    gtk_box_append(GTK_BOX(user->box), create_label("   Locked:"));
    user->locked = gtk_check_button_new();
    gtk_widget_set_tooltip_text(user->locked, "If this option is present, this account is locked by default. This means that the user will not be able to log in from the console.");
    gtk_box_append(GTK_BOX(user->box), user->locked);

    // adding to main_box
    gtk_box_append(GTK_BOX(users_box), user->box);

    return user;
}

void remove_user_from_list(size_t index) {
    if (index >= options.users.count) return;
    
    gtk_box_remove(GTK_BOX(users_box), options.users.list[index]->box);
    free(options.users.list[index]);

    for (size_t i = index; i < options.users.count - 1; ++i) {
        options.users.list[i] = options.users.list[i + 1];
    }
    options.users.count--;
}

void find_and_remove_user_from_list(User *user) {
    for (size_t i = 0; i < options.users.count; i++) {
        if (user == options.users.list[i]) {
            remove_user_from_list(i);
            return;
        }
    }
}

void clear_user_list() {
    for (size_t i = 0; i < options.users.count; i++) {
        gtk_box_remove(GTK_BOX(users_box), options.users.list[i]->box);
        free(options.users.list[i]);
    }
    options.users.list = NULL;
    options.users.capacity = 0;
    options.users.count = 0;
}

User *add_user_to_list() {
    if (options.users.count == options.users.capacity) {
        size_t new_capacity = options.users.capacity == 0 ? 4 : options.users.capacity * 2;
        User **new_list = realloc(options.users.list, new_capacity * sizeof(User *));
        if (!new_list) {
            return NULL;
        }
        options.users.list = new_list;
        options.users.capacity = new_capacity;
    }

    User *user = create_user();

    // delete button for user
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(user->box), spacer);
    GtkWidget *delete_user_btn = gtk_button_new_from_icon_name("user-trash-symbolic");
    g_signal_connect_swapped(delete_user_btn, "clicked", G_CALLBACK(find_and_remove_user_from_list), user);
    gtk_box_append(GTK_BOX(user->box), delete_user_btn);

    options.users.list[options.users.count++] = user;
    return user;
}

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

    // scrollable box
    GtkWidget *users_box_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(users_box_scroll), TRUE);
    gtk_widget_set_vexpand(users_box_scroll, TRUE);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(users_box_scroll), users_box);
    gtk_box_append(GTK_BOX(main_box), users_box_scroll);

    // add user button and remove all button
    GtkWidget *operations_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

    GtkWidget *add_user_btn = gtk_button_new_with_label("Add User");
    gtk_widget_set_hexpand(add_user_btn, TRUE);
    g_signal_connect_swapped(add_user_btn, "clicked", G_CALLBACK(add_user_to_list), NULL);
    gtk_box_append(GTK_BOX(operations_box), add_user_btn);

    GtkWidget *remove_all_btn = gtk_button_new_with_label("Remove All");
    gtk_widget_set_hexpand(remove_all_btn, TRUE);
    g_signal_connect_swapped(remove_all_btn, "clicked", G_CALLBACK(clear_user_list), NULL);
    gtk_box_append(GTK_BOX(operations_box), remove_all_btn);

    GtkWidget *close_btn = gtk_button_new_with_label("Finish");
    gtk_widget_set_hexpand(close_btn, TRUE);
    g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(gtk_window_close), window);
    gtk_box_append(GTK_BOX(operations_box), close_btn);

    gtk_box_append(GTK_BOX(main_box), operations_box);

    gtk_window_present(GTK_WINDOW(window));
}

void setup_users_window_items() {
    users_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    g_object_ref(users_box);

    options.users.list = NULL;
    options.users.count = 0;
    options.users.capacity = 0;
}