#ifndef GLOBALS_H
#define GLOBALS_H

#include <gtk/gtk.h>

struct Packages {
    GListStore *packages;
    GtkWidget *multilib;
    GtkWidget *nocore;
};

struct KickstartOptions {
    char *path;
    GtkWidget *root_enabled;
    GtkWidget *root_password;
    GtkWidget *graphics_mode;
    GtkWidget *locale;
    GtkWidget *layout;
    GtkWidget *selinux;
    GtkWidget *clearpart_all;
    GtkWidget *clearpart_linux;
    GtkWidget *clearpart_none;
    GtkWidget *autopart;
    GtkWidget *bootloader_location;
    GtkWidget *bootloader_options;
    GtkWidget *initial_setup;
    GtkWidget *timezone;
    GtkWidget *after_install;
    GtkWidget *additional_options;
    struct Packages packages;
};

struct OpenedFile {
    char *path;
    FILE *file;
};

extern GtkWidget *window;
extern GtkWidget *label;

extern struct OpenedFile ks_file;
extern struct KickstartOptions options;

#endif
