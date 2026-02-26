#include <gtk/gtk.h>

#include "scripts.h"
#include "../utils/utils.h"
#include "../globals.h"

void new_script_window_with_title(SCRIPT_WINDOW_TYPE type) {
    const char *title;
    GtkWidget *code;
    GtkWidget *interpreter;
    GtkWidget *nochroot;
    if (type == PRE_INSTALL_SCRIPT) {
        title = "Pre Install Script";
        code = options.pre_install.code;
        interpreter = options.pre_install.interpreter;
    } else if (type == POST_INSTALL_SCRIPT) {
        title = "Post Install Script";
        code = options.post_install.code;
        interpreter = options.post_install.interpreter;
        nochroot = options.post_install.nochroot;
    } else {
        return;
    }

    GtkWidget *window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);

    GtkWidget *main_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(main_grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(main_grid), 10);
    gtk_widget_set_margin_start(main_grid, 15);
    gtk_widget_set_margin_end(main_grid, 15);
    gtk_widget_set_margin_top(main_grid, 15);
    gtk_widget_set_margin_bottom(main_grid, 15);

    gtk_window_set_child(GTK_WINDOW(window), main_grid);

    // main part
    GtkWidget *code_label = create_label("Code");
    gtk_widget_set_halign(code_label, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(main_grid), code_label, 0, 0, 1, 1);

    GtkWidget *code_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(code_scroll), TRUE);
    gtk_widget_set_hexpand(code_scroll, TRUE);
    gtk_widget_set_vexpand(code_scroll, TRUE);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(code_scroll), code);
    gtk_grid_attach(GTK_GRID(main_grid), code_scroll, 0, 1, 1, 1);

    // options
    GtkWidget *options_label = create_label("Options");
    gtk_widget_set_halign(options_label, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(main_grid), options_label, 1, 0, 1, 1);

    GtkWidget *options_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *options_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(options_grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(options_grid), 10);
    gtk_widget_set_margin_top(options_grid, 10);

    gtk_grid_attach(GTK_GRID(options_grid), create_label("Interpreter:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(options_grid), interpreter, 1, 0, 1, 1);
    if (type == POST_INSTALL_SCRIPT) {
        gtk_grid_attach(GTK_GRID(options_grid), create_label("Chroot:"), 0, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(options_grid), nochroot, 1, 1, 1, 1);
    }


    GtkWidget *vspacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *close_btn = gtk_button_new_with_label("Finish");
    g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(gtk_window_close), window);
    gtk_widget_set_vexpand(vspacer, TRUE);

    gtk_box_append(GTK_BOX(options_box), options_grid);
    gtk_box_append(GTK_BOX(options_box), vspacer);
    gtk_box_append(GTK_BOX(options_box), close_btn);

    gtk_grid_attach(GTK_GRID(main_grid), options_box, 1, 1, 1, 1);

    gtk_window_present(GTK_WINDOW(window));
}

void setup_scripts_window_items() {
    const char *interpreters[] = {"/usr/bin/sh", "/usr/bin/bash", "/usr/bin/python", NULL};

    // post install script
    options.post_install.code = gtk_text_view_new();
    g_object_ref(options.post_install.code);

    options.post_install.interpreter = gtk_drop_down_new_from_strings(interpreters);
    g_object_ref(options.post_install.interpreter);

    options.post_install.nochroot = gtk_check_button_new_with_label("Disabled");
    gtk_widget_set_tooltip_text(options.post_install.nochroot, "Runs the script outside the chroot environment.");
    g_object_ref(options.post_install.nochroot);

    // pre install script
    options.pre_install.code = gtk_text_view_new();
    g_object_ref(options.pre_install.code);

    options.pre_install.interpreter = gtk_drop_down_new_from_strings(interpreters);
    g_object_ref(options.pre_install.interpreter);
}