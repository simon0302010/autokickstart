#include <stdio.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "../cJSON/cJSON.h"
#include "../globals.h"
#include "../utils/utils.h"
#include "../locale/timezone.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "gtk/gtkdropdown.h"

int load_file(const char *path) {
    char *buffer = NULL;
    FILE *f = fopen(path, "rb");
    if (!f) {
        return 2;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    buffer = malloc(length + 1);
    if (!buffer) {
        fclose(f);
        return 1;
    }
    

    size_t bytes_read = fread(buffer, 1, length, f);
    if (bytes_read != (size_t)length) {
        free(buffer);
        fclose(f);
        return 1;
    }
    buffer[length] = '\0';
    fclose(f);
    
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    
    if (!root) {
        return 1;
    }
    
    cJSON *root_enabled_item = cJSON_GetObjectItem(root, "root_enabled");
    cJSON *root_password_item = cJSON_GetObjectItem(root, "root_password");
    cJSON *graphics_mode_item = cJSON_GetObjectItem(root, "graphics_mode");
    cJSON *locale_item = cJSON_GetObjectItem(root, "locale");
    cJSON *layout_item = cJSON_GetObjectItem(root, "keyboard");
    cJSON *selinux_item = cJSON_GetObjectItem(root, "selinux");
    cJSON *clearpart_item = cJSON_GetObjectItem(root, "clearpart");
    cJSON *autopart_item = cJSON_GetObjectItem(root, "autopart");
    cJSON *bootloader_location_item = cJSON_GetObjectItem(root, "bootloader_location");
    cJSON *bootloader_options_item = cJSON_GetObjectItem(root, "bootloader_options");
    cJSON *initial_setup_item = cJSON_GetObjectItem(root, "initial_setup");
    cJSON *timezone_item = cJSON_GetObjectItem(root, "timezone");
    cJSON *after_install_item = cJSON_GetObjectItem(root, "after_install");
    cJSON *additional_options_item = cJSON_GetObjectItem(root, "additional_options");
    cJSON *packages_item = cJSON_GetObjectItem(root, "packages");
    cJSON *packages_multilib_item = cJSON_GetObjectItem(root, "packages_multilib");
    cJSON *packages_nocore_item = cJSON_GetObjectItem(root, "packages_no_core");
    cJSON *post_install_script_item = cJSON_GetObjectItem(root, "post_install_script");
    cJSON *post_install_interpreter_item = cJSON_GetObjectItem(root, "post_install_interpreter");
    cJSON *post_install_no_chroot_item = cJSON_GetObjectItem(root, "post_install_no_chroot");

    if (root_enabled_item && cJSON_IsBool(root_enabled_item)) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(options.root_enabled), cJSON_IsTrue(root_enabled_item));
    }
    if (root_password_item && root_password_item->valuestring) {
        gtk_editable_set_text(GTK_EDITABLE(options.root_password), root_password_item->valuestring);
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
    if (selinux_item && selinux_item->valueint) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(options.selinux), selinux_item->valueint);
    }
    if (clearpart_item && clearpart_item->valuestring) {
        set_selected_clearpart(clearpart_item->valuestring);
    }
    if (autopart_item && cJSON_IsBool(autopart_item)) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(options.autopart), cJSON_IsTrue(autopart_item));
    }
    if (bootloader_location_item && bootloader_location_item->valueint) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(options.bootloader_location), bootloader_location_item->valueint);
    }
    if (bootloader_options_item && bootloader_options_item->valuestring) {
        gtk_editable_set_text(GTK_EDITABLE(options.bootloader_options), bootloader_options_item->valuestring);
    }
    if (initial_setup_item && cJSON_IsBool(initial_setup_item)) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(options.initial_setup), cJSON_IsTrue(initial_setup_item));
    }
    if (timezone_item && timezone_item->valuestring) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(options.timezone), get_timezone_idx(timezone_item->valuestring, "UTC"));
    }
    if (after_install_item && after_install_item->valueint) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(options.after_install), after_install_item->valueint);
    }
    if (additional_options_item && additional_options_item->valuestring) {
        GtkTextBuffer *additional_options_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(options.additional_options));
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(additional_options_buffer), additional_options_item->valuestring, strlen(additional_options_item->valuestring));
    }
    if (packages_item && cJSON_IsArray(packages_item) && options.packages.packages != NULL) {
        g_list_store_remove_all(options.packages.packages);
        int array_size = cJSON_GetArraySize(packages_item);
        for (int i = 0; i < array_size; i++) {
            cJSON *pkg = cJSON_GetArrayItem(packages_item, i);
            if (pkg && pkg->valuestring) {
                GtkStringObject *str_obj = gtk_string_object_new(pkg->valuestring);
                g_list_store_append(options.packages.packages, str_obj);
                g_object_unref(str_obj);
            }
        }
    }
    if (packages_multilib_item && cJSON_IsBool(packages_multilib_item)) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(options.packages.multilib), packages_multilib_item->valueint);
    }
    if (packages_nocore_item && cJSON_IsBool(packages_nocore_item)) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(options.packages.nocore), packages_nocore_item->valueint);
    }
    if (post_install_script_item && post_install_script_item->valuestring) {
        GtkTextBuffer *post_install_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(options.post_install.code));
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(post_install_buffer), post_install_script_item->valuestring, strlen(post_install_script_item->valuestring));
    }
    if (post_install_interpreter_item && post_install_interpreter_item->valueint) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(options.post_install.interpreter), post_install_interpreter_item->valueint);
    }
    if (post_install_no_chroot_item && cJSON_IsBool(post_install_no_chroot_item)) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(options.post_install.nochroot), packages_nocore_item->valueint);
    }

    cJSON_Delete(root);
    return 0;
}

int save_file(const char *path) {
    cJSON *root = cJSON_CreateObject();

    char clearpart[6];
    get_selected_clearpart(clearpart);

    // used multiple times
    GtkTextIter start, end;

    cJSON_AddBoolToObject(root, "root_enabled", gtk_check_button_get_active(GTK_CHECK_BUTTON(options.root_enabled)));
    cJSON_AddStringToObject(root, "root_password", gtk_editable_get_text(GTK_EDITABLE(options.root_password)));
    cJSON_AddNumberToObject(root, "graphics_mode", gtk_drop_down_get_selected(GTK_DROP_DOWN(options.graphics_mode)));
    cJSON_AddNumberToObject(root, "locale", gtk_drop_down_get_selected(GTK_DROP_DOWN(options.locale)));
    cJSON_AddNumberToObject(root, "keyboard", gtk_drop_down_get_selected(GTK_DROP_DOWN(options.layout)));
    cJSON_AddNumberToObject(root, "selinux", gtk_drop_down_get_selected(GTK_DROP_DOWN(options.selinux)));
    cJSON_AddStringToObject(root, "clearpart", clearpart);
    cJSON_AddBoolToObject(root, "autopart", gtk_check_button_get_active(GTK_CHECK_BUTTON(options.autopart)));
    cJSON_AddStringToObject(root, "bootloader_options", gtk_editable_get_text(GTK_EDITABLE(options.bootloader_options)));
    cJSON_AddNumberToObject(root, "bootloader_location", gtk_drop_down_get_selected(GTK_DROP_DOWN(options.bootloader_location)));
    cJSON_AddBoolToObject(root, "initial_setup", gtk_check_button_get_active(GTK_CHECK_BUTTON(options.initial_setup)));
    const char *tz = get_timezone_from_idx(gtk_drop_down_get_selected(GTK_DROP_DOWN(options.timezone)));
    cJSON_AddStringToObject(root, "timezone", tz ? tz : "UTC");
    cJSON_AddNumberToObject(root, "after_install", gtk_drop_down_get_selected(GTK_DROP_DOWN(options.after_install)));
    GtkTextBuffer *additional_options_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(options.additional_options));
    gtk_text_buffer_get_bounds(additional_options_buffer, &start, &end);
    cJSON_AddStringToObject(root, "additional_options", gtk_text_buffer_get_text(additional_options_buffer, &start, &end, FALSE));
    cJSON_AddBoolToObject(root, "packages_multilib", gtk_check_button_get_active(GTK_CHECK_BUTTON(options.packages.multilib)));
    cJSON_AddBoolToObject(root, "packages_no_core", gtk_check_button_get_active(GTK_CHECK_BUTTON(options.packages.nocore)));
    GtkTextBuffer *post_install_script_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(options.post_install.code));
    gtk_text_buffer_get_bounds(post_install_script_buffer, &start, &end);
    cJSON_AddStringToObject(root, "post_install_script", gtk_text_buffer_get_text(post_install_script_buffer, &start, &end, FALSE));
    cJSON_AddNumberToObject(root, "post_install_interpreter", gtk_drop_down_get_selected(GTK_DROP_DOWN(options.post_install.interpreter)));
    cJSON_AddBoolToObject(root, "post_install_no_chroot", gtk_check_button_get_active(GTK_CHECK_BUTTON(options.post_install.nochroot)));

    // packages
    guint packages_count = g_list_model_get_n_items(G_LIST_MODEL(options.packages.packages));
    const char *packages_list[packages_count];
    for (guint i = 0; i < packages_count; i++) {
        GtkStringObject *pkg = GTK_STRING_OBJECT(g_list_model_get_item(G_LIST_MODEL(options.packages.packages), i));
        packages_list[i] = gtk_string_object_get_string(pkg);
        g_object_unref(pkg);
    }
    cJSON *packages = cJSON_CreateStringArray(packages_list, packages_count);
    cJSON_AddItemToObject(root, "packages", packages);

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
            show_alert(main_window, "Failed to save file");
        } else {
            if (options.path) {
                g_free(options.path);
            }
            options.path = file_path;
        }
        g_object_unref(file);
    }
}

void on_save_file() {
    if (options.path) {
        if (save_file(options.path) != 0) {
            show_alert(main_window, "Failed to save file");
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
        gtk_file_dialog_save(dialog, GTK_WINDOW(main_window), NULL, on_cfg_save_finish, NULL);
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
            show_alert(main_window, "Failed to load file");
        } else {
            if (options.path != NULL) {
                g_free(options.path);
            }
            options.path = file_path;
        }
        g_object_unref(file);
    }
}

void open_file_dialog(GtkWidget *window) {
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
