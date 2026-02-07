#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <unistd.h>

FILE *ks_file;

static void print_input(GtkWidget *widget, gpointer data) {
    const char *text = gtk_editable_get_text(GTK_EDITABLE(data));
    fprintf(ks_file, "%s", text);
    g_print("Input: %s\n", text);
}

static void clear_input(GtkWidget *widget, gpointer data) {
    gtk_editable_delete_text(GTK_EDITABLE(data), 0, -1);
    g_print("Cleared input\n");
}

static bool is_fedora() {
    FILE *os_release = fopen("/etc/os-release", "r");

    if (os_release == NULL) {
        return false;
    }

    char string[50];
    while (fscanf(os_release, "%49s", string) == 1) {
        if (strcasestr(string, "fedora")) {
            fclose(os_release);
            return true;
        }
    }

    fclose(os_release);
    return false;
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *scrolled_window;
    GtkWidget *form_grid;
    GtkWidget *button_box;
    GtkWidget *label;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Auto Kickstart");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);

    // main vertical box
    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_box, 15);
    gtk_widget_set_margin_end(main_box, 15);
    gtk_widget_set_margin_top(main_box, 15);
    gtk_widget_set_margin_bottom(main_box, 15);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // scrolled window
    scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_NEVER,
                                   GTK_POLICY_AUTOMATIC);
    gtk_box_append(GTK_BOX(main_box), scrolled_window);
    gtk_widget_set_vexpand(scrolled_window, TRUE);

    // form grid
    form_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(form_grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(form_grid), 10);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), form_grid);

    // input
    label = gtk_label_new("Username:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(form_grid), label, 0, 2, 1, 1);

    GtkWidget *username_entry = gtk_entry_new();
    gtk_widget_set_hexpand(username_entry, TRUE);
    gtk_grid_attach(GTK_GRID(form_grid), username_entry, 1, 2, 1, 1);

    // button box at bottom
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_box), button_box);

    GtkWidget *open_btn = gtk_button_new_with_label("Open File");
    GtkWidget *build_btn = gtk_button_new_with_label("Build ISO");

    g_signal_connect(open_btn, "clicked", G_CALLBACK(clear_input), username_entry);
    g_signal_connect(build_btn, "clicked", G_CALLBACK(print_input), username_entry);

    gtk_box_append(GTK_BOX(button_box), open_btn);
    gtk_box_append(GTK_BOX(button_box), build_btn);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    ks_file = fopen("/tmp/mbo.ks", "w");
    if (ks_file == NULL) {
        printf("failed to create /tmp/mbo.ks");
        exit(1);
    }

    GtkApplication *app;
    int status;

    app = gtk_application_new("de.simon0302010.autokickstart", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}