#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <unistd.h>

#include "glib.h"
#include "gtk/gtkdropdown.h"
#include "utils/utils.h"
#include "locale/locales.h"
#include "locale/kb.h"
#include "file/file.h"
#include "kickstart/kickstart.h"
#include "globals.h"
#include "locale/timezone.h"

GtkWidget *window;
GtkWidget *label;
struct OpenedFile ks_file;
struct KickstartOptions options;

static void build_iso() {
    g_print("Coming soon\n");
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

static void toogle_root_password_entry() {
    bool using_root_pw = gtk_check_button_get_active(GTK_CHECK_BUTTON(options.root_enabled));
    gtk_widget_set_sensitive(options.root_password, using_root_pw);
    if (!using_root_pw) {
        gtk_editable_set_text(GTK_EDITABLE(options.root_password), "");
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *main_box;
    GtkWidget *scrolled_window;
    GtkWidget *form_grid;
    GtkWidget *button_box;

    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request(spacer, 20, -1);

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

    // root user
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Root User:"), 0, 0, 1, 1);

    GtkWidget *root_user_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_grid_attach(GTK_GRID(form_grid), root_user_box, 1, 0, 1, 1);

    // root enabled checkbox
    GtkWidget *root_enabled = gtk_check_button_new_with_label("Enabled");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(root_enabled), TRUE);
    g_signal_connect_swapped(root_enabled, "toggled", G_CALLBACK(toogle_root_password_entry), NULL);
    gtk_widget_set_tooltip_text(root_enabled, "If this option is disabled, the root account is locked. This means that the root user will not be able to log in from the console.");
    gtk_box_append(GTK_BOX(root_user_box), root_enabled);
    options.root_enabled = root_enabled;

    // root password entry
    gtk_box_append(GTK_BOX(root_user_box), spacer);
    gtk_box_append(GTK_BOX(root_user_box), create_label("Password:"));
    GtkWidget *root_password = gtk_password_entry_new();
    gtk_widget_set_hexpand(root_password, TRUE);
    gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(root_password), TRUE);
    gtk_widget_set_tooltip_text(root_password, "Sets the system’s root password.");
    gtk_box_append(GTK_BOX(root_user_box), root_password);
    options.root_password = root_password;

    // graphics mode dropdown
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Installation Mode:"), 0, 1, 1, 1);

    const char *graphics_modes_array[] = {"Graphical", "Text", "Cmdline", NULL};
    GtkWidget *graphics_mode = gtk_drop_down_new_from_strings(graphics_modes_array);
    gtk_widget_set_hexpand(graphics_mode, TRUE);
    gtk_widget_set_tooltip_text(graphics_mode, "Select the mode the installer will run in");
    gtk_grid_attach(GTK_GRID(form_grid), graphics_mode, 1, 1, 1, 1);
    options.graphics_mode = graphics_mode;

    // locale dropdown
    gtk_grid_attach(GTK_GRID(form_grid), create_label("System Locale:"), 0, 2, 1, 1);

    const char **locale_names = get_locale_names();
    GtkWidget *locale_chooser = gtk_drop_down_new_from_strings(locale_names);
    free(locale_names);
    gtk_widget_set_hexpand(locale_chooser, TRUE);
    gtk_drop_down_set_enable_search(GTK_DROP_DOWN(locale_chooser), TRUE);
    gtk_drop_down_set_search_match_mode(GTK_DROP_DOWN(locale_chooser), GTK_STRING_FILTER_MATCH_MODE_SUBSTRING);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(locale_chooser), get_current_system_locale_index("en_US"));
    gtk_widget_set_tooltip_text(locale_chooser, "Sets the language to use during installation and the default language to use on the installed system.");
    gtk_grid_attach(GTK_GRID(form_grid), locale_chooser, 1, 2, 1, 1);
    options.locale = locale_chooser;

    // layout dropdown
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Keyboard Layout:"), 0, 3, 1, 1);

    const char **layout_names = get_layout_names();
    GtkWidget *layout_chooser = gtk_drop_down_new_from_strings(layout_names);
    free(layout_names);
    gtk_widget_set_hexpand(layout_chooser, TRUE);
    gtk_drop_down_set_enable_search(GTK_DROP_DOWN(layout_chooser), TRUE);
    gtk_drop_down_set_search_match_mode(GTK_DROP_DOWN(layout_chooser), GTK_STRING_FILTER_MATCH_MODE_SUBSTRING);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(layout_chooser), find_current_system_layout_index("us"));
    gtk_widget_set_tooltip_text(layout_chooser, "Sets the keyboard layout to be used on the installed system.");
    gtk_grid_attach(GTK_GRID(form_grid), layout_chooser, 1, 3, 1, 1);
    options.layout = layout_chooser;

    // timezone dropdown
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Timezone:"), 0, 4, 1, 1);

    int timezone_count;
    char **timezones = get_timezones(&timezone_count);
    if (!timezones) {
        g_print("failed to get list of timezones");
        return;
    }
    GtkWidget *timezone_dropdown = gtk_drop_down_new_from_strings((const char * const *)timezones);
    gtk_widget_set_hexpand(timezone_dropdown, TRUE);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(timezone_dropdown), get_current_timezone_idx("UTC"));
    gtk_grid_attach(GTK_GRID(form_grid), timezone_dropdown, 1, 4, 1, 1);

    options.timezone = timezone_dropdown;

    // selinux dropdown
    gtk_grid_attach(GTK_GRID(form_grid), create_label("SELinux:"), 0, 5, 1, 1);

    const char *selinux_options[] = {"Disabled", "Permissive", "Enforcing", NULL};
    GtkWidget *selinux_chooser = gtk_drop_down_new_from_strings(selinux_options);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(selinux_chooser), 2);
    gtk_widget_set_hexpand(selinux_chooser, TRUE);
    gtk_widget_set_tooltip_text(selinux_chooser, "Sets the state of SELinux on the installed system.");
    gtk_grid_attach(GTK_GRID(form_grid), selinux_chooser, 1, 5, 1, 1);
    options.selinux = selinux_chooser;

    // clearpart
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Delete Partitions:"), 0, 6, 1, 1);

    GtkWidget *clearpart_all = gtk_check_button_new_with_label("All");
    GtkWidget *clearpart_linux = gtk_check_button_new_with_label("Linux");
    GtkWidget *clearpart_none = gtk_check_button_new_with_label("None");
    
    gtk_check_button_set_group(GTK_CHECK_BUTTON(clearpart_linux), GTK_CHECK_BUTTON(clearpart_all));
    gtk_check_button_set_group(GTK_CHECK_BUTTON(clearpart_none), GTK_CHECK_BUTTON(clearpart_all));
    
    gtk_check_button_set_active(GTK_CHECK_BUTTON(clearpart_none), TRUE);

    gtk_widget_set_tooltip_text(clearpart_all, "Erases all partitions from the system. This option will erase all disks which can be reached by the installer, including any attached network storage. Use this option with caution.");
    gtk_widget_set_tooltip_text(clearpart_linux, "Erases all Linux partitions.");
    gtk_widget_set_tooltip_text(clearpart_none, "Do not remove any partitions.");

    GtkWidget *clearpart_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(clearpart_box), clearpart_all);
    gtk_box_append(GTK_BOX(clearpart_box), clearpart_linux);
    gtk_box_append(GTK_BOX(clearpart_box), clearpart_none);

    gtk_grid_attach(GTK_GRID(form_grid), clearpart_box, 1, 6, 1, 1);

    options.clearpart_all = clearpart_all;
    options.clearpart_linux = clearpart_linux;
    options.clearpart_none = clearpart_none;

    // autopart option
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Partitioning:"), 0, 7, 1, 1);

    GtkWidget *autopart = gtk_check_button_new_with_label("Automatic");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(autopart), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(autopart), FALSE);
    gtk_grid_attach(GTK_GRID(form_grid), autopart, 1, 7, 1, 1);

    options.autopart = autopart;

    // bootloader
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Bootloader:"), 0, 8, 1, 1);

    GtkWidget *bootloader_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(bootloader_box), create_label("Location:"));

    const char *bootloader_locations[] = {"MBR", "Partition", "None", NULL};
    GtkWidget *bootloader_location_dropdown = gtk_drop_down_new_from_strings(bootloader_locations);
    gtk_widget_set_tooltip_text(bootloader_location_dropdown, "Specifies where the boot record is written. The None option does not install the GRUB bootloader.");
    gtk_box_append(GTK_BOX(bootloader_box), bootloader_location_dropdown);

    gtk_box_append(GTK_BOX(bootloader_box), create_label("Options:"));

    GtkWidget *bootloader_options = gtk_entry_new();
    gtk_widget_set_hexpand(bootloader_options, TRUE);
    gtk_widget_set_tooltip_text(bootloader_options, "Specifies additional kernel parameters. To specify multiple parameters, separate them with spaces.");

    gtk_box_append(GTK_BOX(bootloader_box), bootloader_options);

    gtk_grid_attach(GTK_GRID(form_grid), bootloader_box, 1, 8, 1, 1);

    options.bootloader_options = bootloader_options;
    options.bootloader_location = bootloader_location_dropdown;

    // initial setup
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Initial Setup:"), 0, 9, 1, 1);

    GtkWidget *enable_initial_setup = gtk_check_button_new_with_label("Enabled");
    gtk_widget_set_tooltip_text(enable_initial_setup, "Determine whether the Initial Setup application starts the first time the system is booted. If enabled, the initial-setup package must be installed.");
    gtk_grid_attach(GTK_GRID(form_grid), enable_initial_setup, 1, 9, 1, 1);

    options.initial_setup = enable_initial_setup;

    // packages
    gtk_grid_attach(GTK_GRID(form_grid), create_label("Packages:"), 0, 10, 1, 1);

    GtkWidget *packages_input = gtk_entry_new();
    gtk_widget_set_hexpand(packages_input, TRUE);
    gtk_widget_set_tooltip_text(packages_input, "This describes the software packages to be installed. You can specify packages by environment, group, or by their package names.");
    gtk_grid_attach(GTK_GRID(form_grid), packages_input, 1, 10, 1, 1);

    // button box at bottom
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_box), button_box);

    GtkWidget *open_btn = gtk_button_new_with_label("Open File");
    GtkWidget *save_btn = gtk_button_new_with_label("Save File");
    GtkWidget *build_btn = gtk_button_new_with_label("Build ISO");

    gtk_widget_set_tooltip_text(open_btn, "Open an autokickstart configuration file");
    gtk_widget_set_tooltip_text(save_btn, "Saves the current configuration to a file");
    gtk_widget_set_tooltip_text(build_btn, "Builds an ISO image with the current configuration");

    g_signal_connect_swapped(open_btn, "clicked", G_CALLBACK(open_file_dialog), window);
    g_signal_connect_swapped(save_btn, "clicked", G_CALLBACK(on_save_file), NULL);
    g_signal_connect_swapped(build_btn, "clicked", G_CALLBACK(build_iso), NULL);

    gtk_box_append(GTK_BOX(button_box), open_btn);
    gtk_box_append(GTK_BOX(button_box), save_btn);
    gtk_box_append(GTK_BOX(button_box), build_btn);

    gtk_window_present(GTK_WINDOW(window));
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

    free_timezones();

    if (ks_file.file != NULL) {
        fclose(ks_file.file);
    }
    free(ks_file.path);
    
    if (options.path != NULL) {
        g_free(options.path);
    }

    return status;
}
