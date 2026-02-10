#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <unistd.h>

#include "cJSON/cJSON.h"
#include "gtk/gtkdropdown.h"
#include "utils/utils.h"

struct KickstartOptions {
    char *path;
    GtkWidget *password;
    GtkWidget *username;
    GtkWidget *dropdown;
};

struct OpenedFile {
    char *path;
    FILE *file;
};

GtkWidget *window;
GtkWidget *label;

struct OpenedFile ks_file;
struct KickstartOptions options;

static void build_iso() {
    const char *username = gtk_editable_get_text(GTK_EDITABLE(options.username));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(options.password));
    gpointer dropdown_item = gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.dropdown));
    const char *dropdown = gtk_string_object_get_string(GTK_STRING_OBJECT(dropdown_item));
    g_print("Username:%s, Password: %s, Dropdown: %s\n", username, password, dropdown);
}

static int load_file(const char *path) {
    char *buffer = NULL;
    FILE *f = fopen(path, "rb");
    if (!f) {
        return 2;
    }
    
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    buffer = malloc(length);
    if (!buffer) {
        fclose(f);
        return 1;
    }
    
    fread(buffer, 1, length, f);
    fclose(f);
    
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    
    if (!root) {
        return 1;
    }
    
    cJSON *username_item = cJSON_GetObjectItem(root, "username");
    cJSON *password_item = cJSON_GetObjectItem(root, "password");
    cJSON *dropdown_item = cJSON_GetObjectItem(root, "dropdown");
    
    if (username_item && username_item->valuestring) {
        gtk_editable_set_text(GTK_EDITABLE(options.username), username_item->valuestring);
    }
    if (password_item && password_item->valuestring) {
        gtk_editable_set_text(GTK_EDITABLE(options.password), password_item->valuestring);
    }
    if (dropdown_item && dropdown_item->valueint) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(options.dropdown), dropdown_item->valueint);
    }
    
    cJSON_Delete(root);
    return 0;
}

static int save_file(const char *path) {
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "username", gtk_editable_get_text(GTK_EDITABLE(options.username)));
    cJSON_AddStringToObject(root, "password", gtk_editable_get_text(GTK_EDITABLE(options.password)));
    cJSON_AddNumberToObject(root, "dropdown", gtk_drop_down_get_selected(GTK_DROP_DOWN(options.dropdown)));

    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);

    FILE *f = fopen(path, "w");
    if (!f) {
        free(json_str);
        return 1;
    }

    fputs(json_str, f);
    fclose(f);
    free(json_str);
    return 0;
}

static void on_cfg_save_finish(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GFile *file = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(source_object), res, NULL);

    if (file != NULL) {
        char *file_path = g_file_get_path(file);
        g_print("Saving configuration file to: %s\n", file_path);
        
        if (save_file(file_path) != 0) {
            g_free(file_path);
            show_alert(window, "Failed to load file");
        } else {
            if (options.path) {
                g_free(options.path);
            }
            options.path = file_path;
        }
        g_object_unref(file);
    }
}

static void on_save_file() {
    if (options.path) {
        if (save_file(options.path) != 0) {
            show_alert(window, "Failed to save file");
        }
    } else {
        GtkFileDialog *dialog = gtk_file_dialog_new();
        gtk_file_dialog_set_title(dialog, "Select Configuration File");
        gtk_file_dialog_set_initial_name(dialog, "config.json");
        GtkFileFilter *filter = gtk_file_filter_new();
        gtk_file_filter_add_suffix(filter, "json");
        gtk_file_filter_set_name(filter, "JSON files");
        GListModel *filters = G_LIST_MODEL(g_list_store_new(GTK_TYPE_FILE_FILTER));
        g_list_store_append(G_LIST_STORE(filters), filter);
        gtk_file_dialog_set_filters(dialog, filters);
        g_object_unref(filter);
        g_object_unref(filters);
        gtk_file_dialog_save(dialog, GTK_WINDOW(window), NULL, on_cfg_save_finish, NULL);
        g_object_unref(dialog);
    }
}

static void on_cfg_open_finish(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GFile *file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source_object), res, NULL);

    if (file != NULL) {
        char *file_path = g_file_get_path(file);
        g_print("Loading configuration file from: %s\n", file_path);
        
        if (load_file(file_path) != 0) {
            g_free(file_path);
            show_alert(window, "Failed to load file");
        } else {
            if (options.path != NULL) {
                g_free(options.path);
            }
            options.path = file_path;
        }
        g_object_unref(file);
    }
}

static void open_file_dialog(GtkWidget *window) {
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Select Configuration File");
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_suffix(filter, "json");
    gtk_file_filter_set_name(filter, "JSON files");
    GListModel *filters = G_LIST_MODEL(g_list_store_new(GTK_TYPE_FILE_FILTER));
    g_list_store_append(G_LIST_STORE(filters), filter);
    gtk_file_dialog_set_filters(dialog, filters);
    g_object_unref(filter);
    g_object_unref(filters);
    gtk_file_dialog_open(dialog, GTK_WINDOW(window), NULL, on_cfg_open_finish, NULL);
    g_object_unref(dialog);
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

static GtkWidget *create_label(const char *text) {
    label = gtk_label_new(text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    return label;
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *main_box;
    GtkWidget *scrolled_window;
    GtkWidget *form_grid;
    GtkWidget *button_box;

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
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Username:"), 0, 0, 1, 1);

    GtkWidget *username_entry = gtk_entry_new();
    gtk_widget_set_hexpand(username_entry, TRUE);
    gtk_grid_attach(GTK_GRID(form_grid), username_entry, 1, 0, 1, 1);
    options.username = username_entry;

    // input 2
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Password:"), 0, 1, 1, 1);

    GtkWidget *password_entry = gtk_entry_new();
    gtk_widget_set_hexpand(password_entry, TRUE);
    gtk_grid_attach(GTK_GRID(form_grid), password_entry, 1, 1, 1, 1);
    options.password = password_entry;

    // dropdown
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Mode:"), 0, 2, 1, 1);

    const char *options_array[] = {"Graphical", "Text", NULL};
    GtkWidget *dropdown = gtk_drop_down_new_from_strings(options_array);
    gtk_widget_set_hexpand(dropdown, TRUE);
    gtk_grid_attach(GTK_GRID(form_grid), dropdown, 1, 2, 1, 1);
    options.dropdown = dropdown;

    // button box at bottom
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_box), button_box);

    GtkWidget *open_btn = gtk_button_new_with_label("Open File");
    GtkWidget *save_btn = gtk_button_new_with_label("Save File");
    GtkWidget *build_btn = gtk_button_new_with_label("Build ISO");

    g_signal_connect_swapped(open_btn, "clicked", G_CALLBACK(open_file_dialog), window);
    g_signal_connect_swapped(save_btn, "clicked", G_CALLBACK(on_save_file), NULL);
    g_signal_connect_swapped(build_btn, "clicked", G_CALLBACK(build_iso), NULL);

    gtk_box_append(GTK_BOX(button_box), open_btn);
    gtk_box_append(GTK_BOX(button_box), save_btn);
    gtk_box_append(GTK_BOX(button_box), build_btn);

    gtk_window_present(GTK_WINDOW(window));
}

struct OpenedFile create_temp_ks() {
    struct OpenedFile open_file;

    char *ks_name = rand_str(20);
    if (!ks_name) {
        printf("failed to create random filename for kickstart file\n");
        exit(1);
    }
    char *ks_path = malloc(strlen("/tmp/") + strlen(ks_name) + strlen(".ks") + 1);
    if (!ks_path) {
        printf("failed to allocate string for kickstart file path\n");
        free(ks_name);
        exit(1);
    }
    sprintf(ks_path, "/tmp/%s.ks", ks_name);
    free(ks_name);

    open_file.path = ks_path;
    open_file.file = fopen(ks_path, "w");
    if (open_file.file == NULL) {
        printf("failed to create %s\n", ks_path);
        free(ks_path);
        exit(1);
    }

    return open_file;
}

int main(int argc, char **argv) {
    seed_rng();
    ks_file = create_temp_ks();

    GtkApplication *app;
    int status;

    app = gtk_application_new("de.simon0302010.autokickstart", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    if (ks_file.file != NULL) {
        fclose(ks_file.file);
    }
    g_free(ks_file.path);
    
    if (options.path != NULL) {
        g_free(options.path);
    }

    return status;
}