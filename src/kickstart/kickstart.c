#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "kickstart.h"
#include "../utils/utils.h"
#include "globals.h"
#include "../locale/locales.h"
#include "../locale/kb.h"
#include "gtk/gtkdropdown.h"
#include "locale/timezone.h"

OpenedFile ks_file;

OpenedFile create_temp_ks() {
    OpenedFile open_file;

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

char *write_ks_from_options() {
    ks_file = create_temp_ks();

    GtkTextIter start, end;

    char *graphics_mode = (char *)gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.graphics_mode))));
    for (char *p = graphics_mode; *p; ++p) *p = tolower(*p);
    fprintf(ks_file.file, "%s\n", graphics_mode);

    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.root_enabled))) {
        const char *rootpw = gtk_editable_get_text(GTK_EDITABLE(options.root_password));
        fprintf(ks_file.file, "rootpw --plaintext %s\n", rootpw);
    } else {
        fprintf(ks_file.file, "rootpw --lock\n");
    }

    const char *lang_str = get_locale_id(gtk_drop_down_get_selected(GTK_DROP_DOWN(options.locale)));
    fprintf(ks_file.file, "lang %s\n", lang_str);

    const char *layout_str = get_layout_id(gtk_drop_down_get_selected(GTK_DROP_DOWN(options.layout)));
    fprintf(ks_file.file, "keyboard --xlayouts=%s\n", layout_str);

    const char *timezone_str = get_timezone_from_idx(gtk_drop_down_get_selected(GTK_DROP_DOWN(options.timezone)));
    fprintf(ks_file.file, "timezone %s --utc\n", timezone_str);

    char *selinux_mode = (char *)gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.selinux))));
    for (char *p = selinux_mode; *p; ++p) *p = tolower(*p);
    fprintf(ks_file.file, "selinux --%s\n", selinux_mode);

    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.autopart))) {
        fprintf(ks_file.file, "autopart\n");
    }

    char clearpart[6];
    get_selected_clearpart(clearpart);
    fprintf(ks_file.file, "clearpart --%s\n", clearpart);

    char *bootloader_location = (char *)gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.bootloader_location))));
    for (char *p = bootloader_location; *p; ++p) *p = tolower(*p);
    const char *bootloader_options = gtk_editable_get_text(GTK_EDITABLE(options.bootloader_options));
    if (strlen(bootloader_options) == 0) {
        fprintf(ks_file.file, "bootloader --location=%s\n", bootloader_location);
    } else {
        fprintf(ks_file.file, "bootloader --location=%s --append=\"%s\"\n", bootloader_location, bootloader_options);
    }

    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.initial_setup))) {
        fprintf(ks_file.file, "firstboot --enable\n");
    } else {
        fprintf(ks_file.file, "firstboot --disable\n");
    }

    int after_install_idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(options.after_install));
    const char *after_install_options[] = {"halt", "poweroff", "reboot", "reboot --eject", "shutdown"};
    fprintf(ks_file.file, "%s\n", after_install_options[after_install_idx]);

    GtkTextBuffer *additional_options_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(options.additional_options));
    gtk_text_buffer_get_bounds(additional_options_buffer, &start, &end);
    char *additional_options = gtk_text_buffer_get_text(additional_options_buffer, &start, &end, FALSE);
    fprintf(ks_file.file, "%s\n", additional_options);

    // packages
    fprintf(ks_file.file, "%%packages");
    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.packages.multilib))) {
        fprintf(ks_file.file, " --multilib");
    }
    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.packages.nocore))) {
        fprintf(ks_file.file, " --nocore");
    }
    fprintf(ks_file.file, "\n");
    guint packages_count = g_list_model_get_n_items(G_LIST_MODEL(options.packages.packages));
    for (guint i = 0; i < packages_count; i++) {
        GtkStringObject *pkg = GTK_STRING_OBJECT(g_list_model_get_item(G_LIST_MODEL(options.packages.packages), i));
        fprintf(ks_file.file, "%s\n", gtk_string_object_get_string(pkg));
        g_object_unref(pkg);
    }
    fprintf(ks_file.file, "%%end\n");

    // post install script
    fprintf(ks_file.file, "%%post");
    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.post_install.nochroot))) {
        fprintf(ks_file.file, " --nochroot");
    }
    fprintf(ks_file.file, " --interpreter=%s\n", gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.post_install.interpreter)))));

    GtkTextBuffer *post_install_script_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(options.post_install.code));
    gtk_text_buffer_get_bounds(post_install_script_buffer, &start, &end);
    fprintf(ks_file.file, "%s\n", gtk_text_buffer_get_text(post_install_script_buffer, &start, &end, FALSE));

    fprintf(ks_file.file, "%%end\n");

    if (ks_file.file != NULL) {
        fclose(ks_file.file);
    }

    return ks_file.path;
}