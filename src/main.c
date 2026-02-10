#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <unistd.h>

#include "cJSON/cJSON.h"
#include "gtk/gtkdropdown.h"
#include "gtk/gtkshortcut.h"
#include "utils/utils.h"
#include "locale/locales.h"
#include "locale/kb.h"

struct KickstartOptions {
    char *path;
    GtkWidget *password;
    GtkWidget *username;
    GtkWidget *graphics_mode;
    GtkWidget *locale;
    GtkWidget *layout;
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
    printf("Coming soon");
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
    cJSON *graphics_mode_item = cJSON_GetObjectItem(root, "graphics_mode");
    cJSON *locale_item = cJSON_GetObjectItem(root, "locale");
    cJSON *layout_item = cJSON_GetObjectItem(root, "keyboard");
    
    if (username_item && username_item->valuestring) {
        gtk_editable_set_text(GTK_EDITABLE(options.username), username_item->valuestring);
    }
    if (password_item && password_item->valuestring) {
        gtk_editable_set_text(GTK_EDITABLE(options.password), password_item->valuestring);
    }
    if (graphics_mode_item && graphics_mode_item->valueint) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(options.graphics_mode), graphics_mode_item->valueint);
    }
    if (locale_item && locale_item->valueint) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(options.locale), locale_item->valueint);
    }
    if (layout_item && layout_item->valueint) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(options.layout), layout_item->valueint);
    }

    cJSON_Delete(root);
    return 0;
}

static int save_file(const char *path) {
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "username", gtk_editable_get_text(GTK_EDITABLE(options.username)));
    cJSON_AddStringToObject(root, "password", gtk_editable_get_text(GTK_EDITABLE(options.password)));
    cJSON_AddNumberToObject(root, "graphics_mode", gtk_drop_down_get_selected(GTK_DROP_DOWN(options.graphics_mode)));
    cJSON_AddNumberToObject(root, "locale", gtk_drop_down_get_selected(GTK_DROP_DOWN(options.locale)));
    cJSON_AddNumberToObject(root, "keyboard", gtk_drop_down_get_selected(GTK_DROP_DOWN(options.layout)));

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

    // graphics mode dropdown
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Mode:"), 0, 2, 1, 1);

    const char *graphics_modes_array[] = {"Graphical", "Text", NULL};
    GtkWidget *graphics_mode = gtk_drop_down_new_from_strings(graphics_modes_array);
    gtk_widget_set_hexpand(graphics_mode, TRUE);
    gtk_grid_attach(GTK_GRID(form_grid), graphics_mode, 1, 2, 1, 1);
    options.graphics_mode = graphics_mode;

    // locale dropdown
    gtk_grid_attach(GTK_GRID(form_grid), create_label("System Locale:"), 0, 3, 1, 1);

    const char **locale_names = get_locale_names();
    GtkWidget *locale_chooser = gtk_drop_down_new_from_strings(locale_names);
    free(locale_names);
    gtk_widget_set_hexpand(locale_chooser, TRUE);
    gtk_drop_down_set_enable_search(GTK_DROP_DOWN(locale_chooser), TRUE);
    gtk_drop_down_set_search_match_mode(GTK_DROP_DOWN(locale_chooser), GTK_STRING_FILTER_MATCH_MODE_SUBSTRING);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(locale_chooser), get_current_system_locale_index("en_US"));
    gtk_grid_attach(GTK_GRID(form_grid), locale_chooser, 1, 3, 1, 1);
    options.locale = locale_chooser;

    // layout dropdown
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Keyboard Layout:"), 0, 4, 1, 1);

    const char **layout_names = get_layout_names();
    GtkWidget *layout_chooser = gtk_drop_down_new_from_strings(layout_names);
    free(layout_names);
    gtk_widget_set_hexpand(layout_chooser, TRUE);
    gtk_drop_down_set_enable_search(GTK_DROP_DOWN(layout_chooser), TRUE);
    gtk_drop_down_set_search_match_mode(GTK_DROP_DOWN(layout_chooser), GTK_STRING_FILTER_MATCH_MODE_SUBSTRING);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(layout_chooser), find_current_system_layout_index("us"));
    gtk_grid_attach(GTK_GRID(form_grid), layout_chooser, 1, 4, 1, 1);
    options.layout = layout_chooser;

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