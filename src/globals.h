#ifndef GLOBALS_H
#define GLOBALS_H

#include <gtk/gtk.h>

struct KickstartOptions {
    char *path;
    GtkWidget *root_enabled;
    GtkWidget *root_password;
    GtkWidget *graphics_mode;
    GtkWidget *locale;
    GtkWidget *layout;
    GtkWidget *selinux;
};

struct OpenedFile {
    char *path;
    FILE *file;
};

GtkWidget *window;
GtkWidget *label;

struct OpenedFile ks_file;
struct KickstartOptions options;

#endif