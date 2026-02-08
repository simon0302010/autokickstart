#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <unistd.h>

#include "cJSON/cJSON.h"
#include "utils/utils.h"

struct KickstartOptions {
    GtkWidget *password;
    GtkWidget *username;
};

struct OpenedFile {
    char *path;
    FILE *file;
};

struct OpenedFile ks_file;
struct KickstartOptions options;

static void build_iso() {
    const char *username = gtk_editable_get_text(GTK_EDITABLE(options.username));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(options.password));
    g_print("Username:%s, Password: %s", username, password);
}

static void on_cfg_open_finish(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GFile *file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source_object), res, NULL);

    if (file != NULL) {
        char *file_path = g_file_get_path(file);
        g_print("Path: %s", file_path);
    }
}

static void open_file(GtkWidget *window) {
    GtkFileDialog *dialog;
    dialog = gtk_file_dialog_new();
    gtk_file_dialog_open(dialog, GTK_WINDOW(window), NULL, on_cfg_open_finish, NULL);
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
    options.username = username_entry;

    // input 2
    label = gtk_label_new("Password:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(form_grid), label, 0, 3, 1, 1);

    GtkWidget *password_entry = gtk_entry_new();
    gtk_widget_set_hexpand(password_entry, TRUE);
    gtk_grid_attach(GTK_GRID(form_grid), password_entry, 1, 3, 1, 1);
    options.password = password_entry;

    // button box at bottom
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_box), button_box);

    GtkWidget *open_btn = gtk_button_new_with_label("Open File");
    GtkWidget *build_btn = gtk_button_new_with_label("Build ISO");

    g_signal_connect_swapped(open_btn, "clicked", G_CALLBACK(open_file), window);
    g_signal_connect_swapped(build_btn, "clicked", G_CALLBACK(build_iso), NULL);

    gtk_box_append(GTK_BOX(button_box), open_btn);
    gtk_box_append(GTK_BOX(button_box), build_btn);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    seed_rng();
    char *ks_name = rand_str(20);
    if (!ks_name) {
        printf("failed to create random filename for kickstart file\n");
        return 1;
    }
    char *ks_path = malloc(strlen("/tmp/") + strlen(ks_name) + strlen(".ks") + 1);
    if (!ks_path) {
        printf("failed to allocate string for kickstart file path\n");
        free(ks_name);
        return 1;
    }
    sprintf(ks_path, "/tmp/%s.ks", ks_name);
    free(ks_name);

    ks_file.path = ks_path;
    ks_file.file = fopen(ks_path, "w");
    if (ks_file.file == NULL) {
        printf("failed to create %s\n", ks_path);
        free(ks_path);
        return 1;
    }

    GtkApplication *app;
    int status;

    app = gtk_application_new("de.simon0302010.autokickstart", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}